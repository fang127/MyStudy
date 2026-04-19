package main

import (
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	// r.POST() // 创建资源
	// r.GET() // 获取资源
	// r.PUT() // 更新资源
	// r.DELETE() // 删除资源
	// r.PATCH() // 更新资源的一部分
	// r.ANY() // 支持所有请求方法

	// 路由分组
	// 可以将相关的路由分组在一起，方便管理和维护
	// 例如：用户相关的路由可以分组在一起，订单相关的路由可以分组在一起
	// 这样可以提高代码的可读性和可维护性
	apiGroup := r.Group("api")
	UserGroup(apiGroup)
	loginGroup := r.Group("api")
	LoginGroup(loginGroup)
	r.Run(":8080")
}

func LoginGroup(r *gin.RouterGroup) {
	r.GET("/login", func(ctx *gin.Context) {
		ctx.JSON(200, gin.H{
			"message": "登录页面",
		})
	}) // 登录页面
}

func UserGroup(r *gin.RouterGroup) {
	r.GET("/users", UserView)    // 查询用户列表
	r.POST("/users", UserView)   // 创建用户
	r.DELETE("/users", UserView) // 删除用户
	r.PUT("/users", UserView)    // 更新用户
}

func UserView(c *gin.Context) {
	path := c.Request.URL
	c.JSON(200, gin.H{
		"path":   path,
		"method": c.Request.Method,
	})
}
