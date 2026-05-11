# MyStudy

这是一个个人学习仓库，用来沉淀全栈开发、系统编程和 AI 应用相关的笔记、示例代码与小型实践项目。

## 学习方向

| 方向 | 目录 | 内容概览 | 状态 |
| --- | --- | --- | --- |
| 前端开发 | [frontend](frontend/README.md) | HTML、CSS、JavaScript 基础与浏览器 DOM 示例 | 进行中 |
| C++ 学习 | [cpp](cpp/README.md) | C++ 基础、并发组件、网络通信、性能组件与手撕题 | 进行中 |
| Golang 学习 | [golang](golang/README.md) | Gin、Gorm、RPC/gRPC 方向的 Web 后端实践 | 进行中 |
| AI 应用开发 | [ai](ai/README.md) | LLM、RAG、Agent、向量数据库与 AI 应用工程化 | 规划中 |

## 目录结构

```text
MyStudy/
├── ai/                # AI 应用开发学习
├── cpp/               # C++ 学习与系统组件实践
├── frontend/          # 前端基础学习
└── golang/            # Go 后端开发学习
```

## 重点专题

### C++

- [ThreadPool](cpp/ThreadPool/README.md)：C++11 线程池实现与 `future` 异步任务封装。
- [gRPC](cpp/gRPC/README.md)：C++ gRPC 同步、异步 CQ、Callback Reactor、TLS/mTLS 示例。
- [IM 高性能消息队列](cpp/IM高性能消息队列/README.md)：基于 Folly MPMCQueue 的分片无锁消息队列。
- [时间轮](cpp/时间轮/README.md)：面向 IM Session 超时管理的线程安全时间轮。
- [连接池](cpp/连接池/README.md)：支持统一适配层的 C++17 通用连接池。
- [手撕系列](cpp/手撕系列/README.md)：常见 C++ 基础组件、算法和并发题练习。
- [笔试算法](cpp/笔试算法/README.md)：笔试算法题整理目录。

### Golang

- [Gin-Study](golang/Gin-Study/README.md)：Gin 框架路由、参数、响应、绑定与中间件学习。
- [Gorm-Study](golang/Gorm-Study/README.md)：Gorm 模型、CRUD、关联、事务与自定义类型实践。
- [grpc-demo](golang/grpc-demo/README.md)：RPC/gRPC 入门示例。

### Frontend

- [HTML](frontend/HTML/README.md)：HTML 标签、表单、CSS 引入、DOM 事件等页面练习。
- [CSS](frontend/CSS/README.md)：公共样式与基础样式实验。
- [JavaScript](frontend/JavaScript/README.md)：JS 语法、对象、函数、闭包、Promise、async 等基础。

## 使用方式

1. 每个一级目录都有自己的 README，适合作为该方向的入口。
2. 专题目录中的 README 记录项目目标、文件说明、构建或运行方式。
3. 构建类项目优先查看对应目录的 `CMakeLists.txt`、`go.mod` 或示例入口文件。
