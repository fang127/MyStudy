package res

import (
	"github.com/gin-gonic/gin"
)

type Response struct {
	Code int         `json:"code"`
	Msg  string      `json:"msg"`
	Data interface{} `json:"data"`
}

var codeMap = map[int]string{
	1001: "权限错误",
	1002: "角色错误",
}

func response(c *gin.Context, code int, msg string, data any) {
	c.JSON(200, Response{
		Code: code,
		Msg:  msg,
		Data: data,
	})
}

func OK(c *gin.Context, data any, msg string) {
	response(c, 0, msg, data)
}

func OkWithMsg(c *gin.Context, msg string) {
	OK(c, gin.H{}, msg)
}

func OkWithData(c *gin.Context, data any) {
	OK(c, data, "success")
}

func Fail(c *gin.Context, code int, data any, msg string) {
	response(c, code, msg, data)
}

func FailWithMsg(c *gin.Context, msg string) {
	response(c, 1001, msg, nil)
}

func FailWithCode(c *gin.Context, code int) {
	msg, ok := codeMap[code]
	if !ok {
		msg = "服务错误"
	}
	response(c, code, msg, nil)
}
