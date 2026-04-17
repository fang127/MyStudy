package main

import "github.com/gin-gonic/gin"

type Response struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
	Data any    `json:"data"`
}

func Index(c *gin.Context) {
	c.JSON(200, Response{
		Code: 0,
		Msg:  "成功",
		Data: map[string]any{},
	})
}

func main() {
	// 1. 初始化
	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()
	// 2. 挂载路由
	r.GET("/index", func(c *gin.Context) {
	})
	// 3. 绑定端口，开始运行
	r.Run(":8080")
}
