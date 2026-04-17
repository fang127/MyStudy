package main

import (
	"log"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.GET("", func(c *gin.Context) {
		// 设置响应头，告诉浏览器这是一个文件下载
		c.Header("Content-Type", "application/octet-stream")            // 表示这是一个文件流，唤起浏览器的下载功能
		c.Header("Content-Disposition", "attachment; filename=main.go") // 指定下载文件的名称

		c.File("main.go")
	})
	err := r.Run(":8080")
	if err != nil {
		log.Fatal(err)
	}
}
