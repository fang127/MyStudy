package main

import "github.com/gin-gonic/gin"

func main() {
	r := gin.Default()
	// 1. 查询参数
	r.GET("/index", func(c *gin.Context) {
		type User struct {
			Name string `form:"name"`
			Age  int    `form:"age"`
		}
		var user User
		err := c.ShouldBindQuery(&user)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, gin.H{"name": user.Name, "age": user.Age})
	})

	// 2. 路径参数
	r.GET("/user/:id/:name", func(c *gin.Context) {
		type User struct {
			ID   string `uri:"id"`
			Name string `uri:"name"`
		}
		var user User
		err := c.ShouldBindUri(&user)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, gin.H{"id": user.ID, "name": user.Name})
	})

	// 3. 表单参数
	r.POST("/form", func(c *gin.Context) {
		type User struct {
			Name string `form:"name"`
			Age  int    `form:"age"`
		}
		var user User
		// ShouldBind 会根据 HTTP 方法和 Content-Type 请求头自动选择绑定引擎
		// 注意：ShouldBind会根据请求的Content-Type自动选择绑定方式
		// 如果Content-Type是application/x-www-form-urlencoded或multipart/form-data，则绑定表单参数
		err := c.ShouldBind(&user)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, gin.H{"name": user.Name, "age": user.Age})
	})

	// 4. JSON 参数
	r.POST("/json", func(c *gin.Context) {
		type User struct {
			Name string `json:"name"`
			Age  int    `json:"age"`
		}
		var user User
		err := c.ShouldBindJSON(&user)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}
		c.JSON(200, gin.H{"name": user.Name, "age": user.Age})
	})

	// 5. 请求头
	r.POST("header", func(ctx *gin.Context) {
		type User struct {
			Token       string `header:"Token"`
			ContentType string `header:"Content-Type"`
		}
		var user User
		err := ctx.ShouldBindHeader(&user)
		if err != nil {
			ctx.JSON(400, gin.H{"error": err.Error()})
			return
		}
		ctx.JSON(200, gin.H{"token": user.Token, "content_type": user.ContentType})
	})

	r.Run(":8080")
}
