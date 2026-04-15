package main

import "github.com/gin-gonic/gin"

func main() {
	// 1. 初始化
	r := gin.Default()
	// 2. 挂载路由
	r.GET("/index", func(c *gin.Context) {
	})
}
