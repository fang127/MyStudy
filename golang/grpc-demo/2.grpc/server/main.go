package main

import (
	"context"
	"fmt"
	"io"
	"log"
	"net"
	"time"

	calculatorpb "grpc-demo/2.grpc/pb"

	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type calculatorServer struct {
	// 嵌入 UnimplementedCalculatorServiceServer 是新版 gRPC-Go 推荐写法：
	// 以后 proto 新增方法时，旧服务端也能以“未实现”错误安全运行。
	calculatorpb.UnimplementedCalculatorServiceServer
}

func (s *calculatorServer) Add(ctx context.Context, req *calculatorpb.AddRequest) (*calculatorpb.AddResponse, error) {
	result := req.GetA() + req.GetB()
	log.Printf("[简单 RPC] 收到 %d + %d = %d", req.GetA(), req.GetB(), result)

	return &calculatorpb.AddResponse{Result: result}, nil
}

func (s *calculatorServer) SumClientStream(stream grpc.ClientStreamingServer[calculatorpb.NumberRequest, calculatorpb.SumResponse]) error {
	var total int32

	for {
		// Recv 会阻塞等待客户端发送下一条消息。
		req, err := stream.Recv()
		if err == io.EOF {
			// io.EOF 表示客户端已经调用 CloseAndRecv/CloseSend 结束发送。
			// 客户端流式 RPC 的响应只返回一次，所以这里用 SendAndClose。
			log.Printf("[客户端流式 RPC] 客户端发送结束，累计结果：%d", total)
			return stream.SendAndClose(&calculatorpb.SumResponse{Total: total})
		}
		if err != nil {
			return err
		}

		total += req.GetNumber()
		log.Printf("[客户端流式 RPC] 收到数字：%d，当前累计：%d", req.GetNumber(), total)
	}
}

func (s *calculatorServer) RangeServerStream(req *calculatorpb.RangeRequest, stream grpc.ServerStreamingServer[calculatorpb.NumberResponse]) error {
	if req.GetStart() > req.GetEnd() {
		return status.Error(codes.InvalidArgument, "start 不能大于 end")
	}

	log.Printf("[服务端流式 RPC] 开始推送范围：%d 到 %d", req.GetStart(), req.GetEnd())
	for number := req.GetStart(); number <= req.GetEnd(); number++ {
		// Send 每调用一次，客户端就可以 Recv 到一条响应。
		if err := stream.Send(&calculatorpb.NumberResponse{Number: number}); err != nil {
			return err
		}

		time.Sleep(300 * time.Millisecond)
	}

	return nil
}

func (s *calculatorServer) Chat(stream grpc.BidiStreamingServer[calculatorpb.ChatRequest, calculatorpb.ChatResponse]) error {
	for {
		// 双向流可以边收边发。这里用“收到一条，回复一条”的形式方便观察。
		req, err := stream.Recv()
		if err == io.EOF {
			// 客户端关闭发送方向后，服务端返回 nil，表示响应流也正常结束。
			log.Println("[双向流式 RPC] 客户端结束聊天")
			return nil
		}
		if err != nil {
			return err
		}

		reply := fmt.Sprintf("服务端收到 %s 的消息：%s", req.GetName(), req.GetMessage())
		log.Printf("[双向流式 RPC] %s", reply)

		if err := stream.Send(&calculatorpb.ChatResponse{Message: reply}); err != nil {
			return err
		}
	}
}

func main() {
	// 在本地监听 50051 端口，等待客户端连接。
	listener, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("监听端口失败：%v", err)
	}
	// 创建 gRPC 服务器实例，并注册 CalculatorService 服务实现。
	grpcServer := grpc.NewServer()
	calculatorpb.RegisterCalculatorServiceServer(grpcServer, &calculatorServer{})

	log.Println("gRPC 服务已启动，监听地址：:50051")
	// 启动 gRPC 服务器，阻塞等待客户端连接和请求。
	if err := grpcServer.Serve(listener); err != nil {
		log.Fatalf("启动 gRPC 服务失败：%v", err)
	}
}
