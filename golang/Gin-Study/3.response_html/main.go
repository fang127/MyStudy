package main

import (
	"log"
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.LoadHTMLGlob("templates/*")
	// r.LoadHTMLFiles("templates/index.html") 该方法加载某个文件
	r.GET("", func(c *gin.Context) {
		// 第二个参数是文件名，不一定是html
		// 只写文件名,所以需要给Gin提供文件路径
		c.HTML(http.StatusOK, "index.html", map[string]any{
			"title": "MyBlog",
		})
	})
	err := r.Run(":8080")
	if err != nil {
		log.Fatal(err)
	}
}
