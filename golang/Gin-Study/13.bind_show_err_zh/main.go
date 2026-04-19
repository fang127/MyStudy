package main

import (
	"fmt"
	"net"
	"reflect"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gin-gonic/gin/binding"
	"github.com/go-playground/locales/zh"
	ut "github.com/go-playground/universal-translator"
	"github.com/go-playground/validator/v10"
	zh_translations "github.com/go-playground/validator/v10/translations/zh"
)

var trans ut.Translator // 翻译器

func init() {
	// 1. 创建通用翻译器，默认语言为中文
	uni := ut.New(zh.New())
	// 2. 获取中文翻译器实例
	trans, _ = uni.GetTranslator("zh")
	// 3. 获取Gin内置的验证器引擎，并做类型断言
	v, ok := binding.Validator.Engine().(*validator.Validate)
	// 4. 如果类型断言成功，注册中文默认翻译
	if ok {
		_ = zh_translations.RegisterDefaultTranslations(v, trans)
	}

	// 5. 注册一个函数，获取结构体字段的标签作为翻译的字段名
	v.RegisterTagNameFunc(func(field reflect.StructField) string {
		label := field.Tag.Get("label")
		if label == "" {
			label = field.Name
		}

		name := field.Tag.Get("json")
		if name == "" {
			name = field.Name
		}
		return fmt.Sprintf("%s---%s", name, label)
	})

	// 6. 自定义验证规则和翻译
	// fip是一个自定义的验证标签，尽量不要与内置标签冲突
	v.RegisterValidation("fip", func(fl validator.FieldLevel) bool {
		fmt.Println("Field(): ", fl.Field())
		fmt.Println("FieldName(): ", fl.FieldName())
		fmt.Println("StructFieldName(): ", fl.StructFieldName())
		fmt.Println("Parent(): ", fl.Parent())
		fmt.Println("Top(): ", fl.Top())
		fmt.Println("Param(): ", fl.Param())

		ip, ok := fl.Field().Interface().(string)
		if ok && ip != "" {
			// 这里可以添加自定义的IP地址验证逻辑，例如检查是否为特定的IP地址
			ipObj := net.ParseIP(ip)
			// 如果解析成功，说明是一个有效的IP地址，可以进一步检查是否与参数匹配
			if ipObj != nil {
				return ip == fl.Param()
			}
		}

		return true // 如果字段不是字符串类型或者为空，则不进行验证，直接返回true
	})
}

type User struct {
	Name  string `json:"name" binding:"required" label:"用户名"`
	Ip    string `json:"ip" binding:"fip=1234" label:"IP地址"`
	Email string `json:"email" binding:"required,email" label:"邮箱"`
}

func ValidateErr(err error) any {
	// 1. 将错误类型断言为validator.ValidationErrors
	errs, ok := err.(validator.ValidationErrors)
	if !ok {
		return err.Error()
	}
	// 2. 遍历错误列表，翻译每个错误信息，并将它们连接成一个字符串返回
	var m = map[string]any{}
	for _, e := range errs {
		msg := e.Translate(trans)
		list := strings.Split(msg, "---") // 切割出字段名和错误信息
		// 3. 如果错误信息中包含自定义标签（例如fip），可以根据需要进行特殊处理
		if e.Tag() == "fip" {
			msg = fmt.Sprintf("%s必须是特定的IP地址", e.Field())
			m[strings.Split(e.Field(), "---")[0]] = msg
			continue
		}
		m[list[0]] = list[1] // 将字段名作为键，错误信息作为值存入map
	}
	return m
}

func main() {
	r := gin.Default()

	r.POST("/users", func(ctx *gin.Context) {
		var user User
		if err := ctx.ShouldBindJSON(&user); err != nil {
			ctx.JSON(400, ValidateErr(err))
			return
		}
		ctx.JSON(200, user)
	})

	r.Run(":8080")
}
