# IM 高性能消息队列

这是一个面向 IM 场景的 C++20 高性能消息队列示例，核心实现基于 `folly::MPMCQueue`，通过分片队列降低竞争，并用高低水位实现背压控制。

## 学习重点

- 有界 MPMC 队列的使用方式。
- 分片队列设计：按 `user_id` 路由到固定分片，保证单用户消息有序。
- 背压控制：高水位暂停写入，低水位恢复写入。
- 使用原子变量维护近似队列深度和暂停状态。
- 生产者、消费者和业务消息模型的基本组织。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| [include/message.hpp](include/message.hpp) | 单条业务消息模型，包含用户、会话、序号、载荷和入队时间。 |
| [include/high_performance_queue.hpp](include/high_performance_queue.hpp) | 分片无锁消息队列核心实现。 |
| [src/main.cpp](src/main.cpp) | 队列使用演示入口。 |
| [CMakeLists.txt](CMakeLists.txt) | CMake 构建配置。 |

## 构建

前置依赖：

- CMake >= 3.20
- C++20 编译器
- Folly
- Threads

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./build/im_mq_demo
```

## 设计说明

队列由多个分片组成，每个分片维护一个有界 `folly::MPMCQueue<Message>`。生产者按 `user_id % shard_count` 选择分片，因此同一用户的消息始终进入同一分片，有利于维持用户内顺序。

当分片深度达到 `high_watermark_per_shard` 时，该分片进入暂停状态；当消费者出队后深度回落到 `low_watermark_per_shard` 以下时恢复写入。这个机制用于在高并发写入时保护下游消费能力。
