# 无锁编程

这个目录整理无锁数据结构和安全内存回收相关代码，用于学习 C++ 原子操作、CAS、ABA 问题和并发容器设计。

## 文件说明

| 文件 | 主题 | 说明 |
| --- | --- | --- |
| [include/spsc_ring_queue.h](include/spsc_ring_queue.h) | SPSC 环形队列 | 单生产者单消费者无锁队列。 |
| [include/mpmc_bounded_queue.h](include/mpmc_bounded_queue.h) | MPMC 有界队列 | 多生产者多消费者有界队列。 |
| [include/lock_free_stack_deferred.h](include/lock_free_stack_deferred.h) | 无锁栈 | 使用延迟回收思路的无锁栈。 |
| [include/lock_free_stack_hazard.h](include/lock_free_stack_hazard.h) | 无锁栈 | 使用 Hazard Pointer 保护节点生命周期。 |
| [include/hazard_pointer.h](include/hazard_pointer.h) | Hazard Pointer | 防止并发读写中节点被提前释放。 |
| [include/epoch_reclaimer.h](include/epoch_reclaimer.h) | Epoch 回收 | 基于 epoch 的延迟回收机制。 |
| [include/epoch_forward_list.h](include/epoch_forward_list.h) | 链表 | 结合 epoch 回收的前向链表示例。 |
| [include/ref_counted_forward_list.h](include/ref_counted_forward_list.h) | 链表 | 基于引用计数的并发链表示例。 |

## 学习重点

- `std::atomic` 的基本使用和内存序。
- CAS 循环和失败重试策略。
- 无锁结构中的节点生命周期管理。
- ABA 问题、Hazard Pointer、Epoch 回收的应用场景。
- SPSC、MPMC 队列在约束条件和实现复杂度上的差异。

## 注意事项

无锁代码的正确性依赖严格的并发语义，阅读时建议同时关注：

- 哪些对象可能被多个线程同时访问。
- 节点释放是否可能早于其它线程读取。
- `memory_order` 是否与数据发布、读取、回收流程匹配。
- 压测和单元测试是否覆盖多线程竞争场景。
