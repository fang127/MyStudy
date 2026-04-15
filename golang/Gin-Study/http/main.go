package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
)

type Response struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
	Data any    `json:"data"`
}

func Index(writer http.ResponseWriter, request *http.Request) {
	fmt.Println(request.Method, request.URL.String())
	if request.Method != "GET" {
		byteData, _ := io.ReadAll(request.Body)
		fmt.Println(byteData)
	}
	fmt.Println(request.Header)
	byteData, err := json.Marshal(Response{
		Code: 0,
		Msg:  "success",
		Data: map[string]any{},
	})
	if err != nil {
		log.Fatal(err)
	}
	writer.Write(byteData)
}

func main() {
	http.HandleFunc("/", Index)

	http.ListenAndServe("0.0.0.0:8080", nil)
	fmt.Println("http server running 127.0.0.1:8080")
}

// 1. 原生http参数解析不方便
// 2. 路由不太清晰，一个URL，需要在一个函数内测试多种请求方法
// 3. 响应的处理比较原理
