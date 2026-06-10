package gee

import (
	"log"
	"net/http"
)

// HandlerFunc定义了gee框架中使用的请求处理程序的类型，参数和返回值与http.HandlerFunc相同
type HandlerFunc func(c *Context)

// Engine是gee框架的核心结构，包含一个路由表router，用于存储HTTP方法和URL模式与处理程序函数之间的映射关系
type Engine struct {
	router *router
}

// New函数创建一个新的Engine实例，并初始化路由表router为一个空的map
func New() *Engine {
	return &Engine{router: newRouter()}
}

// addRoute方法是Engine结构体的一个方法，用于向路由表router中添加一个新的路由规则。它接受HTTP方法、URL模式和处理程序函数作为参数，并将它们存储在路由表中，同时打印出添加的路由信息
func (engine *Engine) addRoute(method string, pattern string, handler HandlerFunc) {
	log.Printf("Route %4s - %s", method, pattern)
	engine.router.addRoute(method, pattern, handler)
}

func (engine *Engine) GET(pattern string, handler HandlerFunc) {
	engine.router.addRoute("GET", pattern, handler)
}

func (engine *Engine) POST(pattern string, handler HandlerFunc) {
	engine.router.addRoute("POST", pattern, handler)
}

func (engine *Engine) Run(addr string) (err error) {
	return http.ListenAndServe(addr, engine)
}

// ServeHTTP方法是Engine实现http.Handler接口的关键方法
func (engine *Engine) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	c := newContext(w, req)
	engine.router.handle(c)
}
