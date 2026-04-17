package main

import (
	"log"

	"Gin-Study/2.response_json/res"
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	r.GET("/login", func(c *gin.Context) {
		//c.JSON(200, gin.H{
		//	"code": 0,
		//	"msg":  "success",
		//	"data": gin.H{},
		//})
		//res.OK(data, "")
		//res.OkWithMsg("success")
		//res.OkWithData(data)
		res.OkWithMsg(c, "登陆成功")
	})

	r.GET("/users", func(c *gin.Context) {
		res.OkWithData(c, map[string]interface{}{
			"name": "harry",
		})
	})

	r.POST("/users", func(c *gin.Context) {
		res.FailWithMsg(c, "参数错误")
	})
	err := r.Run(":8080")
	if err != nil {
		log.Fatal(err)
	}
}
