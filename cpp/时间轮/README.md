# 高性能时间轮

这是一个 C++17 线程安全时间轮实现，适用于 IM Session、连接保活、心跳超时等需要大量定时过期管理的场景。

## 学习重点

- 环形槽位时间轮的基本结构。
- 使用分片索引降低锁竞争。
- 刷新会话超时时间的均摊 O(1) 路径。
- Tick 线程和过期回调线程的分离。
- `shared_ptr` / `weak_ptr` 在定时管理中的生命周期处理。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| [include/time_wheel.h](include/time_wheel.h) | 时间轮核心模板实现。 |
| [src/main.cpp](src/main.cpp) | 基础演示程序。 |
| [tests/unit_tests.cpp](tests/unit_tests.cpp) | 单元测试。 |
| [tests/perf_test.cpp](tests/perf_test.cpp) | 性能测试。 |
| [CMakeLists.txt](CMakeLists.txt) | CMake 构建和测试配置。 |

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./build/time_wheel_demo
```

## 测试

```bash
ctest --test-dir build --output-on-failure
```

也可以直接运行：

```bash
./build/time_wheel_unit_tests
./build/time_wheel_perf_test
```

## 设计说明

时间轮通过固定数量的槽位表达未来一段时间的过期窗口。每次刷新 Session 时，根据当前 tick 计算未来槽位，把 Session 放入对应槽中，同时在分片索引里记录位置。Tick 线程推进到某个槽位时，将可能过期的 Session 放入过期队列，再由回调线程执行实际业务逻辑，避免 Tick 线程被业务处理阻塞。
