package main

import "github.com/gin-gonic/gin"

func main() {
	r := gin.Default()
	// 访问路径 http://localhost:8080/st/index.html
	// 第一个参数是访问路径，第二个参数是静态文件所在的目录
	// 注意：静态文件的路径不能和其他路由冲突，否则会被路由覆盖掉
	r.Static("st", "static")
	r.Run(":8080")
}
