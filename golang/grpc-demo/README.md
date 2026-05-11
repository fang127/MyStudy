# Go RPC/gRPC 学习示例

这个目录用两个阶段学习 Go 里的远程调用：

1. `1.origin_grpc`：Go 标准库 `net/rpc`，先理解 RPC 的基本模型。
2. `2.grpc`：gRPC + Protobuf，学习同步、异步和四种传输模式。

## 1. 原生 RPC：`net/rpc`

原生 RPC 的核心思想是：服务端注册一个对象，客户端通过“服务名.方法名”远程调用这个对象的方法。

服务端在 `1.origin_grpc/server/main.go` 中注册服务：

```go
// 注册服务
rpc.Register(new(Server))

// 将 RPC 服务绑定到 HTTP 协议上
rpc.HandleHTTP()

// 监听端口
listen, err := net.Listen("tcp", ":8080")

// 启动 HTTP 服务
http.Serve(listen, nil)
```

服务端暴露的远程方法必须符合 `net/rpc` 的方法签名约定：

```go
func (s *Server) Add(req *Req, res *Res) error {
	res.Num = req.Num1 + req.Num2
	return nil
}
```

客户端在 `1.origin_grpc/client/main.go` 中通过 `DialHTTP` 建立连接，然后可以同步调用：

```go
err = client.Call("Server.Add", res, &reply)
if err != nil {
	log.Fatalln(err)
}
```

同步调用会阻塞当前 goroutine，直到服务端返回结果。

原生 RPC 也提供异步调用：

```go
asyncCall := client.Go("Server.Add", res, &reply, nil)
replyCall := <-asyncCall.Done
if replyCall.Error != nil {
	log.Fatalln(replyCall.Error)
}
```

`client.Go` 会立即返回，真正的调用结果通过 `Done` channel 通知。

运行方式：

```bash
cd /home/harry/CODE/MyStudy/golang/grpc-demo

# Terminal A
go run ./1.origin_grpc/server

# Terminal B
go run ./1.origin_grpc/client
```

## 2. 从原生 RPC 到 gRPC

`net/rpc` 很适合入门，但它更偏 Go 语言内部生态：

- 接口依赖 Go 的结构体和方法命名。
- 跨语言能力弱。
- 不直接提供客户端流、服务端流、双向流这些现代 RPC 场景。

gRPC 的做法是先写 `.proto` 文件，把接口契约独立出来：

```proto
service CalculatorService {
  rpc Add(AddRequest) returns (AddResponse);
  rpc SumClientStream(stream NumberRequest) returns (SumResponse);
  rpc RangeServerStream(RangeRequest) returns (stream NumberResponse);
  rpc Chat(stream ChatRequest) returns (stream ChatResponse);
}
```

然后通过 `protoc` 生成 Go 代码。服务端实现生成出来的接口，客户端调用生成出来的 client。

gRPC 默认基于 HTTP/2，所以天然支持“流式传输”。这也是它和普通一次请求一次响应模型最大的差异之一。

## 3. gRPC 四种传输模式

### 3.1 简单 RPC

简单 RPC 是最像普通函数调用的一种模式：客户端发一次请求，服务端回一次响应。

```proto
rpc Add(AddRequest) returns (AddResponse);
```

服务端实现：

```go
func (s *calculatorServer) Add(ctx context.Context, req *calculatorpb.AddRequest) (*calculatorpb.AddResponse, error) {
	result := req.GetA() + req.GetB()
	return &calculatorpb.AddResponse{Result: result}, nil
}
```

客户端同步调用：

```go
resp, err := client.Add(ctx, &calculatorpb.AddRequest{A: 1, B: 2})
```

gRPC-Go 的普通方法本身是阻塞调用。如果想做异步，可以用 goroutine + channel 包一层：

```go
resultCh := make(chan addResult, 1)
go func() {
	resp, err := client.Add(ctx, &calculatorpb.AddRequest{A: 3, B: 4})
	resultCh <- addResult{resp: resp, err: err}
}()

result := <-resultCh
```

### 3.2 客户端流式 RPC

客户端流式适合“客户端连续上传多条数据，服务端最后返回一个结果”的场景，比如上传分片、批量提交、累计求和。

```proto
rpc SumClientStream(stream NumberRequest) returns (SumResponse);
```

客户端不断 `Send`：

```go
for _, number := range []int32{10, 20, 30, 40} {
	stream.Send(&calculatorpb.NumberRequest{Number: number})
}

resp, err := stream.CloseAndRecv()
```

服务端不断 `Recv`，直到读到 `io.EOF`：

```go
for {
	req, err := stream.Recv()
	if err == io.EOF {
		return stream.SendAndClose(&calculatorpb.SumResponse{Total: total})
	}
	total += req.GetNumber()
}
```

### 3.3 服务端流式 RPC

服务端流式适合“客户端请求一次，服务端连续推送多条结果”的场景，比如分页推送、日志订阅、进度通知。

```proto
rpc RangeServerStream(RangeRequest) returns (stream NumberResponse);
```

服务端多次 `Send`：

```go
for number := req.GetStart(); number <= req.GetEnd(); number++ {
	stream.Send(&calculatorpb.NumberResponse{Number: number})
}
```

客户端循环 `Recv`：

```go
for {
	resp, err := stream.Recv()
	if err == io.EOF {
		break
	}
	log.Println(resp.GetNumber())
}
```

### 3.4 双向流式 RPC

双向流式适合客户端和服务端都要持续收发的场景，比如聊天、实时协作、语音识别、实时指标处理。

```proto
rpc Chat(stream ChatRequest) returns (stream ChatResponse);
```

客户端可以一边发送：

```go
stream.Send(&calculatorpb.ChatRequest{
	Name:    "client",
	Message: "你好",
})
```

也可以一边接收：

```go
resp, err := stream.Recv()
```

服务端同样可以循环 `Recv`，再按需要 `Send`：

```go
req, err := stream.Recv()
reply := "服务端收到：" + req.GetMessage()
stream.Send(&calculatorpb.ChatResponse{Message: reply})
```

## 4. 生成 gRPC 代码

`.proto` 文件位于：

```text
2.grpc/proto/calculator.proto
```

在 `grpc-demo` 目录下执行：

```bash
protoc --go_out=. --go-grpc_out=. 2.grpc/proto/calculator.proto
```

生成文件位于：

```text
2.grpc/pb/calculator.pb.go
2.grpc/pb/calculator_grpc.pb.go
```

如果重新生成后发现 `RangeServerStream` 仍然不是服务端流式，请确认 proto 中这一行包含 `stream`：

```proto
rpc RangeServerStream(RangeRequest) returns (stream NumberResponse);
```

## 5. 运行 gRPC 示例

启动服务端：

```bash
cd /home/harry/CODE/MyStudy/golang/grpc-demo
go run ./2.grpc/server
```

启动客户端：

```bash
cd /home/harry/CODE/MyStudy/golang/grpc-demo
go run ./2.grpc/client
```

客户端会依次演示：

- 简单 RPC 同步调用。
- 简单 RPC 异步调用。
- 客户端流式 RPC。
- 服务端流式 RPC。
- 双向流式 RPC。

## 6. 目录结构

```text
grpc-demo
├── 1.origin_grpc
│   ├── client
│   ├── server
│   └── main.go
├── 2.grpc
│   ├── client
│   ├── pb
│   ├── proto
│   └── server
├── go.mod
├── go.sum
└── README.md
```
