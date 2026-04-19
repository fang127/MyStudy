package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

func Home(c *gin.Context) {
	fmt.Println("Home")
	c.String(200, "Home")
}

func Middleware1(c *gin.Context) {
	fmt.Println("Middleware1 before")
	c.Next()
	fmt.Println("Middleware1 after")
}

func Middleware2(c *gin.Context) {
	fmt.Println("Middleware2 before")
	c.Next()
	// c.Abort()                        // 如果调用了 c.Abort()，后续的中间件和处理函数将不会被执行，但 Middleware1 的响应部分和 Middleware2 的响应部分仍然会被执行
	fmt.Println("Middleware2 after") // 如果调用了 c.Abort()，这行代码将仍然执行
}

func GlobalMiddleware(c *gin.Context) {
	fmt.Println("GlobalMiddleware before")
	c.Next()
	fmt.Println("GlobalMiddleware after")
	value, exists := c.Get("key") // 在中间件中获取数据
	if exists {
		fmt.Println("Value from MiddlewareWithData:", value)
	} else {
		fmt.Println("Key not found in context")
	}
}

func MiddlewareWithData(c *gin.Context) {
	c.Set("key", "value") // 在中间件中设置数据
	c.Next()
}

func MiddlewareGetData(c *gin.Context) {
	value, exists := c.Get("key") // 在中间件中获取数据
	if exists {
		fmt.Println("Value from MiddlewareWithData:", value)
	} else {
		fmt.Println("Key not found in context")
	}
	c.Next()
}

func main() {
	r := gin.Default()

	// 全局中间件：对所有路由生效
	// 全局中间件会在每个请求开始时执行，并在请求结束时执行，可以用于日志记录、错误处理、认证等功能
	// 全局中间件优先于局部中间件执行，因为它们在路由处理之前被注册
	r.Use(GlobalMiddleware)

	// 先走 Middleware1 请求部分，再走 Middleware2 请求部分，最后走 Home，然后依次走 Middleware2 响应部分和 Middleware1 响应部分
	// 通过 c.Next() 来控制中间件的执行顺序
	// 局部中间件：只对特定的路由生效
	r.GET("/", Middleware1, Middleware2, Home)

	// 中间件传递数据：可以通过 c.Set() 和 c.Get() 来在中间件之间传递数据
	r.GET("/data", MiddlewareWithData, MiddlewareGetData, func(c *gin.Context) {
		c.String(200, "Data route")
	})

	r.Run(":8080")
}
