package main

import (
	"fmt"
	"log"

	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.POST("/users", func(ctx *gin.Context) {
		// 1. 获取文件(表单字段名为 "file"，这里是单文件)
		file, err := ctx.FormFile("file")
		// 也可以获取多个文件，使用 ctx.MultipartForm() 来获取所有文件
		// 表单字段名为 "files"
		// files, err := ctx.MultipartForm()

		if err != nil {
			ctx.String(400, "获取文件失败: %v", err)
			return
		}
		fmt.Println(file.Filename) // 文件名
		fmt.Println(file.Size)     // 文件大小，单位为字节

		// 可以打开文件进行读取，但这里我们直接保存文件到服务器
		// f = file.Open()                // 打开文件
		// defer f.Close()                // 关闭文件
		// content, err := ioutil.ReadAll(f) // 读取文件内容

		// 2. 保存文件到服务器
		err = ctx.SaveUploadedFile(file, "./uploads/"+file.Filename)
		if err != nil {
			ctx.String(500, "保存文件失败: %v", err)
			return
		}

		ctx.String(200, "文件上传成功: %s", file.Filename)
	})
	err := r.Run(":8080")
	if err != nil {
		log.Fatal(err)
	}
}
