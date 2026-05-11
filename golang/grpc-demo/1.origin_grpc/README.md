# 原生 RPC 示例

这个目录使用 Go 标准库 `net/rpc` 演示最基础的 RPC 调用流程：服务端注册方法，客户端通过 HTTP RPC 连接并发起同步、异步调用。

## 文件说明

| 文件或目录 | 说明 |
| --- | --- |
| [server/main.go](server/main.go) | RPC 服务端，注册 `Server.Add` 并监听 `:8080`。 |
| [client/main.go](client/main.go) | RPC 客户端，演示同步 `Call` 和异步 `Go` 调用。 |
| [main.go](main.go) | 早期单文件服务端示例，功能与 `server/main.go` 类似。 |

## 运行

Terminal A：

```bash
go run ./1.origin_grpc/server
```

Terminal B：

```bash
go run ./1.origin_grpc/client
```

## 学习重点

- `rpc.Register` 注册服务对象。
- `rpc.HandleHTTP` 将 RPC 服务绑定到 HTTP。
- `net.Listen` 监听 TCP 端口。
- `rpc.DialHTTP` 建立客户端连接。
- `client.Call` 同步调用和 `client.Go` 异步调用。
