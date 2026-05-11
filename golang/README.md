# Golang 开发学习

记录 Go Web 后端、数据库访问和 RPC 通信相关学习内容。

## 学习内容

- Go module、项目结构与基础工程组织。
- Gin Web 框架：路由、参数处理、响应、文件上传、绑定校验和中间件。
- Gorm ORM：模型定义、迁移、CRUD、原生 SQL、关联关系、事务和自定义类型。
- RPC/gRPC：服务注册、调用流程、客户端/服务端通信模型。

## 目录索引

| 目录 | 主题 | 说明 |
| --- | --- | --- |
| [Gin-Study](Gin-Study/README.md) | Gin 框架 | 从初始化到路由、中间件、参数绑定的系列示例。 |
| [Gorm-Study](Gorm-Study/README.md) | Gorm ORM | 数据库连接、模型、单表操作、关联、事务等示例。 |
| [grpc-demo](grpc-demo/README.md) | RPC/gRPC | Go RPC/gRPC 入门示例，目前包含原生 RPC 服务端。 |

## 运行提示

进入具体项目目录后执行：

```bash
go mod tidy
go run ./具体章节目录
```

部分示例依赖本地服务：

- Gin 示例默认使用 HTTP 服务端口，按章节源码中的 `Run` 配置访问。
- Gorm 示例需要可用的 MySQL 环境，可参考 [Gorm-Study/docker-compose.yaml](Gorm-Study/docker-compose.yaml)。
