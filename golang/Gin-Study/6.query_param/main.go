package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.GET("/", func(c *gin.Context) {
		// c.Query()方法获取查询参数，如果没有则返回空字符串
		age := c.Query("age")
		// 查询name参数，如果没有则使用默认值Guest
		name := c.DefaultQuery("name", "Guest")
		// c.QueryArray()方法获取查询参数的数组形式，如果没有则返回空数组
		arr := c.QueryArray("key")
		c.String(200, "Hello %s", name)
		fmt.Println(name, age, arr)

	})
	err := r.Run(":8080")
	if err != nil {
		panic(err)
	}
}
