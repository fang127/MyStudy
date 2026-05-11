# C++11 ThreadPool

一个轻量级 C++11 线程池实现，用来学习任务队列、工作线程、`std::future` 和可变参数模板的配合方式。

## 学习重点

- 使用固定数量工作线程消费任务队列。
- 使用 `std::mutex`、`std::condition_variable` 实现线程同步。
- 通过 `std::packaged_task` 和 `std::future` 获取异步任务结果。
- 通过 `enqueue` 接收任意可调用对象和参数。
- 在线程池析构时通知工作线程退出并回收资源。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| [ThreadPool.h](ThreadPool.h) | 线程池核心实现。 |
| [example.cpp](example.cpp) | 基础使用示例。 |

## 基本用法

```cpp
#include "ThreadPool.h"

#include <iostream>

int main()
{
    ThreadPool pool(4);

    auto result = pool.enqueue([](int answer) {
        return answer;
    }, 42);

    std::cout << result.get() << std::endl;
}
```

## 编译运行

```bash
g++ -std=c++11 -pthread example.cpp -o example
./example
```

## 适合继续扩展的方向

- 支持任务优先级。
- 支持动态调整线程数量。
- 增加任务队列长度限制和提交失败策略。
- 增加统计信息，例如排队任务数、完成任务数和工作线程状态。
