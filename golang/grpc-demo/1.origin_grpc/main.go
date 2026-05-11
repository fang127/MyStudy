package main

import (
	"log"
	"net"
	"net/http"
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

func (s *Server) Add(req *Req, res *Res) error {
	res.Num = req.Num1 + req.Num2
	return nil
}

func main() {
	// 注册服务
	rpc.Register(new(Server))
	// 将RPC服务绑定到HTTP协议上
	rpc.HandleHTTP()
	// 监听端口
	listen, err := net.Listen("tcp", ":8080")
	if err != nil {
		log.Fatalln(err)
		return
	}
	// 启动HTTP服务
	http.Serve(listen, nil)
}
