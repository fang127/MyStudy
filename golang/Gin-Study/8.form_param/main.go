package main

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.POST("/users", func(c *gin.Context) {
		// 使用 c.GetPostForm 获取 POST 请求中的参数，返回值是参数值和一个布尔值，表示参数是否存在
		// 和PostForm的区别在于，如果参数不存在，GetPostForm会返回空字符串和false，而PostForm会返回空字符串
		// name, exits:= c.GetPostForm("name")
		// 使用 c.PostForm 获取 POST 请求中的参数
		name := c.PostForm("name")
		message := c.PostForm("message")
		// 使用 c.DefaultPostForm 获取 POST 请求中的参数，如果参数不存在则返回默认值
		age := c.DefaultPostForm("age", "18")
		c.String(http.StatusOK, "name: %s, age: %s, message: %s", name, age, message)
	})
	r.Run(":8080")
}
