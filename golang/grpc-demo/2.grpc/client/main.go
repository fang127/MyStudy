package main

import (
	"context"
	"io"
	"log"
	"time"

	calculatorpb "grpc-demo/2.grpc/pb"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type addResult struct {
	resp *calculatorpb.AddResponse
	err  error
}

func main() {
	conn, err := grpc.NewClient("localhost:50051", grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		log.Fatalf("连接 gRPC 服务失败：%v", err)
	}
	defer conn.Close()

	client := calculatorpb.NewCalculatorServiceClient(conn)

	callAddSync(client)
	callAddAsync(client)
	callClientStream(client)
	callServerStream(client)
	callBidirectionalStream(client)
}

func callAddSync(client calculatorpb.CalculatorServiceClient) {
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	// 简单 RPC 的同步调用：Add 会阻塞，直到拿到服务端响应或发生错误。
	resp, err := client.Add(ctx, &calculatorpb.AddRequest{A: 1, B: 2})
	if err != nil {
		log.Fatalf("[简单 RPC - 同步] 调用失败：%v", err)
	}

	log.Printf("[简单 RPC - 同步] 1 + 2 = %d", resp.GetResult())
}

func callAddAsync(client calculatorpb.CalculatorServiceClient) {
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	resultCh := make(chan addResult, 1)
	go func() {
		// gRPC-Go 的普通客户端方法本身是阻塞调用。
		// 想做“异步调用”时，通常用 goroutine + channel 包一层。
		resp, err := client.Add(ctx, &calculatorpb.AddRequest{A: 3, B: 4})
		resultCh <- addResult{resp: resp, err: err}
	}()

	result := <-resultCh
	if result.err != nil {
		log.Fatalf("[简单 RPC - 异步] 调用失败：%v", result.err)
	}

	log.Printf("[简单 RPC - 异步] 3 + 4 = %d", result.resp.GetResult())
}

func callClientStream(client calculatorpb.CalculatorServiceClient) {
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	// SumClientStream 是一个客户端流式 RPC，返回一个 stream 对象，客户端通过它发送多条请求消息，最后等待服务端返回一个响应。
	stream, err := client.SumClientStream(ctx)
	if err != nil {
		log.Fatalf("[客户端流式 RPC] 创建流失败：%v", err)
	}

	for _, number := range []int32{10, 20, 30, 40} {
		// Send 每调用一次，就向服务端发送一条请求消息。
		log.Printf("[客户端流式 RPC] 发送数字：%d", number)
		if err := stream.Send(&calculatorpb.NumberRequest{Number: number}); err != nil {
			log.Fatalf("[客户端流式 RPC] 发送失败：%v", err)
		}
	}

	// CloseAndRecv 会关闭客户端发送方向，并等待服务端返回最终响应。
	resp, err := stream.CloseAndRecv()
	if err != nil {
		log.Fatalf("[客户端流式 RPC] 接收汇总结果失败：%v", err)
	}

	log.Printf("[客户端流式 RPC] 累计结果：%d", resp.GetTotal())
}

func callServerStream(client calculatorpb.CalculatorServiceClient) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	stream, err := client.RangeServerStream(ctx, &calculatorpb.RangeRequest{Start: 1, End: 5})
	if err != nil {
		log.Fatalf("[服务端流式 RPC] 调用失败：%v", err)
	}

	for {
		// Recv 每次读取服务端推送的一条消息。
		resp, err := stream.Recv()
		if err == io.EOF {
			// io.EOF 表示服务端已经发送完毕，流正常结束。
			log.Println("[服务端流式 RPC] 服务端推送结束")
			return
		}
		if err != nil {
			log.Fatalf("[服务端流式 RPC] 接收失败：%v", err)
		}

		log.Printf("[服务端流式 RPC] 收到数字：%d", resp.GetNumber())
	}
}

func callBidirectionalStream(client calculatorpb.CalculatorServiceClient) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	stream, err := client.Chat(ctx)
	if err != nil {
		log.Fatalf("[双向流式 RPC] 创建流失败：%v", err)
	}

	done := make(chan error, 1)
	go func() {
		messages := []string{"你好", "我正在学习 gRPC", "双向流很适合实时通信"}
		for _, message := range messages {
			// 双向流里，客户端可以多次 Send，同时服务端也可以多次 Send。
			log.Printf("[双向流式 RPC] 客户端发送：%s", message)
			if err := stream.Send(&calculatorpb.ChatRequest{Name: "client", Message: message}); err != nil {
				done <- err
				return
			}
			time.Sleep(300 * time.Millisecond)
		}

		// CloseSend 只关闭客户端发送方向，仍然可以继续 Recv 服务端剩余响应。
		done <- stream.CloseSend()
	}()

	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			log.Println("[双向流式 RPC] 服务端响应结束")
			break
		}
		if err != nil {
			log.Fatalf("[双向流式 RPC] 接收失败：%v", err)
		}

		log.Printf("[双向流式 RPC] 服务端回复：%s", resp.GetMessage())
	}

	if err := <-done; err != nil {
		log.Fatalf("[双向流式 RPC] 发送失败：%v", err)
	}
}
