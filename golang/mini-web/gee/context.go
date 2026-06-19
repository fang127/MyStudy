package gee

import (
	"encoding/json"
	"fmt"
	"net/http"
)

type H map[string]interface{}

type Context struct {
	// 原始请求和响应
	Writer http.ResponseWriter
	Req    *http.Request

	// 请求信息
	Path   string
	Method string
	Params map[string]string

	// 响应状态码
	StatusCode int

	// 中间件
	handlers []HandlerFunc
	index    int

	// engine 指针，方便访问引擎实例
	engine *Engine
}

func (c *Context) Param(key string) string {
	value, _ := c.Params[key]
	return value
}

// 构造函数，创建一个新的 Context 实例
func newContext(w http.ResponseWriter, req *http.Request) *Context {
	return &Context{
		Writer: w,
		Req:    req,
		Path:   req.URL.Path,
		Method: req.Method,
		index:  -1,
	}
}

func (c *Context) Next() {
	c.index++
	len := len(c.handlers)
	for ; c.index < len; c.index++ {
		c.handlers[c.index](c)
	}
}

func (c *Context) Fail(code int, err string) {
	c.index = len(c.handlers)
	c.JSON(code, H{"message": err})
}

// 从 POST 请求中获取表单数据
func (c *Context) PostForm(key string) string {
	return c.Req.FormValue(key)
}

// 从 URL 查询参数中获取数据
func (c *Context) Query(key string) string {
	return c.Req.URL.Query().Get(key)
}

// 设置响应状态码
func (c *Context) Status(code int) {
	c.StatusCode = code
	c.Writer.WriteHeader(code)
}

// 设置响应头
func (c *Context) SetHeader(key string, value string) {
	c.Writer.Header().Set(key, value)
}

// 格式化字符串响应
func (c *Context) String(code int, format string, values ...interface{}) {
	c.SetHeader("Content-Type", "text/plain")
	c.Status(code)
	c.Writer.Write([]byte(fmt.Sprintf(format, values...)))
}

// 格式化 JSON 响应
func (c *Context) JSON(code int, obj interface{}) {
	c.SetHeader("Content-Type", "application/json")
	c.Status(code)
	encoder := json.NewEncoder(c.Writer)
	if err := encoder.Encode(obj); err != nil {
		http.Error(c.Writer, err.Error(), 500)
	}
}

// 格式化二进制数据响应
func (c *Context) Data(code int, data []byte) {
	c.Status(code)
	c.Writer.Write(data)
}

// 格式化 HTML 响应
func (c *Context) HTML(code int, name string, data interface{}) {
	c.SetHeader("Content-Type", "text/html")
	c.Status(code)
	if err := c.engine.htmlTemplates.ExecuteTemplate(c.Writer, name, data); err != nil {
		c.Fail(500, err.Error())
	}
}
