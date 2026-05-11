# C++ 通用连接池

这是一个线程安全的 C++17 通用连接池实现。连接池本身不绑定具体后端，调用方只需要实现 `IConnection` 接口并提供连接工厂，就可以接入数据库连接、缓存连接、HTTP 连接等资源。

## 功能特性

1. 线程安全：使用互斥锁 + 条件变量，支持多线程并发获取/归还连接。
2. 连接复用：连接通过 RAII Handle 自动归还，减少反复创建销毁。
3. 最大连接数限流：严格 `maxConnections` 上限，防止打爆下游。
4. 空闲连接回收：空闲超过 `idleTimeout` 自动关闭（保底 `minConnections`）。
5. 健康检查：获取连接时执行 `ping()`，失效连接自动重连。
6. 动态扩缩容：高并发自动扩容（不超上限），低负载自动缩容。
7. 获取超时：`acquire(timeout)` 超时返回，避免业务线程无限阻塞。
8. 异常重连：连接/心跳失败后自动重试重连（次数和退避可配置）。
9. 防泄漏强制回收：租借超过 `leakTimeout` 的连接会被后台线程回收槽位。

## 项目结构

- [connection_pool.h](connection_pool.h)：连接池核心接口、配置、统计、RAII Handle。
- [connection_pool.cpp](connection_pool.cpp)：连接池核心实现（并发控制、维护线程、扩缩容、回收逻辑）。
- [main.cpp](main.cpp)：演示程序，使用 `MockDbConnection` 模拟连接创建、心跳、重连和泄漏回收。
- [CMakeLists.txt](CMakeLists.txt)：构建配置。

## 架构分层
~~~
        IConnection（抽象连接）
                ↓
    PooledConnection（包装连接+元数据）
                ↓
ConnectionPool（核心池管理：获取/归还/维护）
                ↓
    Handle（RAII句柄：自动归还，防泄漏）
~~~

## 接入方式

连接池核心只感知 `IConnection`：

1. 业务连接实现 `connect()`、`close()`、`ping()`。
2. 创建连接池时传入 `ConnectionFactory`。
3. 使用 `acquire()` 获取 RAII `Handle`。
4. `Handle` 析构或手动 `release()` 时，连接自动归还连接池。

## 快速开始

### 1. 构建

~~~bash
cmake -S . -B build
cmake --build build -j
~~~

### 2. 运行

~~~bash
./build/connection_pool_demo
~~~

程序会创建多个工作线程并周期输出连接池统计，例如：

- 总连接数
- 空闲连接数
- 使用中连接数
- 泄漏回收次数
- 重连成功/失败
- 创建失败次数

## 配置说明

在 [main.cpp](main.cpp#L10) 中通过 `ConnectionPool::Config` 配置：

- `minConnections`：最小连接数
- `maxConnections`：最大连接数
- `acquireTimeout`：获取连接超时
- `idleTimeout`：空闲回收超时
- `maintenanceInterval`：后台维护周期
- `leakTimeout`：泄漏判定超时
- `reconnectAttempts`：重连尝试次数
- `reconnectBackoff`：重连退避间隔

## 如何接入真实连接

思路是把真实客户端对象封装进 `IConnection`。

示例（伪代码）：

~~~cpp
class MySqlConnection : public IConnection {
public:
    bool connect() override { /* mysql_real_connect */ }
    void close() noexcept override { /* mysql_close */ }
    bool ping() override { /* mysql_ping */ }
};

class RedisConnection : public IConnection {
public:
    bool connect() override { /* redisConnect */ }
    void close() noexcept override { /* redisFree */ }
    bool ping() override { /* PING command */ }
};
~~~

然后创建连接池：

~~~cpp
auto pool = ConnectionPool::Create(
    cfg,
    []() { return std::make_unique<MySqlConnection>(); });
~~~

## 注意事项

1. 示例里故意制造了少量“泄漏”以演示强制回收机制，生产代码不要这样做。
2. 当你需要按业务路由到不同后端时，可以在工厂函数中扩展策略（按权重、按标签、按请求上下文分配）。
3. 若要提升观测性，建议在连接池中增加日志钩子或导出监控指标。
