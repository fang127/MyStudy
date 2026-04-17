package main

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	router := gin.Default()

	// 该路由将匹配 /user/john，但不匹配 /user/john/ 或 /user/john/send
	router.GET("/user/:name", func(c *gin.Context) {
		name := c.Param("name")
		c.String(http.StatusOK, "Hello %s", name)
	})

	// 该路由将匹配 /user/john/send 和 /user/john/，但不匹配 /user/john
	router.GET("/user/:name/*action", func(c *gin.Context) {
		name := c.Param("name")
		action := c.Param("action")
		message := name + " is " + action
		c.String(http.StatusOK, message)
	})

	router.Run(":8080")
}
