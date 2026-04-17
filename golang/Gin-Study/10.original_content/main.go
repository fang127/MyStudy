package main

import (
	"bytes"
	"fmt"
	"io"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.POST("/index", func(ctx *gin.Context) {
		// 读取请求体数据，读了之后，请求体就被关闭了，后续就无法再读取了
		byteData, err := io.ReadAll(ctx.Request.Body)
		if err != nil {
			ctx.String(500, "read body error")
			return
		}
		fmt.Println(string(byteData))
		// 这次读取name参数，读取不到了，因为请求体已经被关闭了
		// 解决办法：在第一次读取请求体数据之后，重新构造一个新的请求体
		ctx.Request.Body = io.NopCloser(bytes.NewReader(byteData)) // 重新构造一个新的请求体，使用之前读取到的数据
		name := ctx.PostForm("name")
		fmt.Println(name)
		ctx.String(200, "ok")
	})
	r.Run(":8080")
}
