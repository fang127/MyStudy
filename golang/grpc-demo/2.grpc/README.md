# gRPC 计算器示例

这个目录用 `CalculatorService` 演示 gRPC 的四种传输模式：

| 方法 | 类型 | 说明 |
| --- | --- | --- |
| `Add` | 简单 RPC | 客户端发送两个数字，服务端返回加法结果。 |
| `SumClientStream` | 客户端流式 | 客户端连续发送数字，服务端最终返回累计求和。 |
| `RangeServerStream` | 服务端流式 | 客户端发送范围，服务端连续返回范围内数字。 |
| `Chat` | 双向流式 | 客户端和服务端持续发送、接收消息。 |

## 文件说明

| 路径 | 说明 |
| --- | --- |
| `proto/calculator.proto` | Protobuf 服务和消息定义。 |
| `pb/calculator.pb.go` | Protobuf 消息生成代码。 |
| `pb/calculator_grpc.pb.go` | gRPC 客户端和服务端接口生成代码。 |
| `server/main.go` | gRPC 服务端实现。 |
| `client/main.go` | gRPC 客户端调用示例。 |

## 生成代码

在 `grpc-demo` 目录下执行：

```bash
protoc --go_out=. --go-grpc_out=. 2.grpc/proto/calculator.proto
```

服务端流式定义要带 `stream`：

```proto
rpc RangeServerStream(RangeRequest) returns (stream NumberResponse);
```

## 运行

Terminal A：

```bash
go run ./2.grpc/server
```

Terminal B：

```bash
go run ./2.grpc/client
```

客户端会依次输出简单 RPC 同步调用、简单 RPC 异步调用、客户端流式、服务端流式、双向流式的结果。
