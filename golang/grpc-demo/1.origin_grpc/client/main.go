package main

import (
	"log"
	"net/rpc"
)

type Server struct {
}

type Req struct {
	Num1 int
	Num2 int
}

type Res struct {
	Num int
}

func main() {
	res := Req{Num1: 1, Num2: 2}
	// DialHTTP 连接到 RPC 服务器
	client, err := rpc.DialHTTP("tcp", ":8080")
	if err != nil {
		log.Fatalln(err)
	}
	var reply Res
	// 同步调用
	err = client.Call("Server.Add", res, &reply)
	if err != nil {
		log.Fatalln(err)
	}
	log.Printf("result: %d", reply.Num)

	// 异步调用
	res = Req{Num1: 3, Num2: 4}
	asyncCall := client.Go("Server.Add", res, &reply, nil)
	replyCall := <-asyncCall.Done
	if replyCall.Error != nil {
		log.Fatalln(replyCall.Error)
	}
	log.Printf("result: %d", reply.Num)
}
