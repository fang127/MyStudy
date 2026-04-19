package main

import "github.com/gin-gonic/gin"

func main() {
	r := gin.Default()
	r.POST("/json", func(ctx *gin.Context) {
		type User struct {
			// binding标签中可以使用required、min、max等规则来验证字段的值
			// 这里的name字段必须满足：必填、长度在3到5之间
			Name string `json:"name" binding:"required,min=3,max=5"`
			Age  int    `json:"age"`
			Pwd  string `json:"pwd" binding:"required"`
			// eqfield规则用于验证两个字段的值是否相等，这里要求re_pwd字段的值必须与pwd字段的值相等
			RePwd string `json:"re_pwd" binding:"required,eqfield=Pwd"`
		}
		var user User
		err := ctx.ShouldBindBodyWithJSON(&user)
		if err != nil {
			ctx.JSON(400, gin.H{"error": err.Error()})
			return
		}
		ctx.JSON(200, user)
	})
	err := r.Run(":8080")
	if err != nil {
		panic(err)
	}
}
