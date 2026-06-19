package gee

import (
	"log"
	"net/http"
	"path"
	"strings"
	"text/template"
)

// HandlerFunc定义了gee框架中使用的请求处理程序的类型，参数和返回值与http.HandlerFunc相同
type HandlerFunc func(c *Context)

// Engine是gee框架的核心结构，包含一个路由表router，用于存储HTTP方法和URL模式与处理程序函数之间的映射关系
type (
	Engine struct {
		*RouterGroup                     // 通过匿名字段实现继承，使Engine具有RouterGroup的所有方法
		router        *router            // 路由表
		groups        []*RouterGroup     // 存储所有的路由组
		htmlTemplates *template.Template // HTML模板
		funcMap       template.FuncMap   // 模板函数映射
	}

	RouterGroup struct {
		prefix      string        // 前缀
		middlewares []HandlerFunc // 中间件
		parent      *RouterGroup  // 父级路由组
		engine      *Engine       // 共享Engine实例
	}
)

// New函数创建一个新的Engine实例，并初始化路由表router为一个空的map
func New() *Engine {
	engine := &Engine{router: newRouter()}
	engine.RouterGroup = &RouterGroup{engine: engine}  // 初始化RouterGroup并将Engine实例赋值给它
	engine.groups = []*RouterGroup{engine.RouterGroup} // 将根路由组添加到groups切片中
	return engine
}

func Default() *Engine {
	engine := New()
	engine.Use(Logger(), Recovery())
	return engine
}

func (group *RouterGroup) Use(middlewares ...HandlerFunc) {
	group.middlewares = append(group.middlewares, middlewares...)
}

// Group方法用于创建一个新的RouterGroup，并将其添加到Engine的groups切片中。新组的前缀是父组前缀加上新前缀，父级路由组设置为调用该方法的RouterGroup，新的RouterGroup共享同一个Engine实例
func (group *RouterGroup) Group(prefix string) *RouterGroup {
	engine := group.engine
	newGroup := &RouterGroup{
		prefix: group.prefix + prefix, // 新组的前缀是父组前缀加上新前缀
		parent: group,                 // 设置父级路由组
		engine: engine,                // 共享Engine实例
	}
	engine.groups = append(engine.groups, newGroup) // 将新组添加到Engine的groups切片中
	return newGroup
}

// addRoute方法是Engine结构体的一个方法，用于向路由表router中添加一个新的路由规则。它接受HTTP方法、URL模式和处理程序函数作为参数，并将它们存储在路由表中，同时打印出添加的路由信息
func (group *RouterGroup) addRoute(method string, comp string, handler HandlerFunc) {
	pattern := group.prefix + comp
	log.Printf("Route %4s - %s", method, pattern)
	group.engine.router.addRoute(method, pattern, handler)
}

func (group *RouterGroup) GET(pattern string, handler HandlerFunc) {
	group.addRoute("GET", pattern, handler)
}

func (group *RouterGroup) POST(pattern string, handler HandlerFunc) {
	group.addRoute("POST", pattern, handler)
}

// 创建静态Handler
func (group *RouterGroup) createStaticHandler(relativePath string, fs http.FileSystem) HandlerFunc {
	absolutePath := path.Join(group.prefix, relativePath)
	fileServer := http.StripPrefix(absolutePath, http.FileServer(fs))
	return func(c *Context) {
		file := c.Param("filepath")
		if _, err := fs.Open(file); err != nil {
			c.Status(http.StatusNotFound)
			return
		}
		fileServer.ServeHTTP(c.Writer, c.Req)
	}
}

// 静态文件服务
func (group *RouterGroup) Static(relativePath string, root string) {
	handler := group.createStaticHandler(relativePath, http.Dir(root))
	urlPattern := path.Join(relativePath, "/*filepath")
	group.GET(urlPattern, handler)
}

func (engine *Engine) SetFuncMap(funcMap template.FuncMap) {
	engine.funcMap = funcMap
}

func (engine *Engine) LoadHTMLGlob(pattern string) {
	engine.htmlTemplates = template.Must(template.New("").Funcs(engine.funcMap).ParseGlob(pattern))
}

func (engine *Engine) Run(addr string) (err error) {
	return http.ListenAndServe(addr, engine)
}

// ServeHTTP方法是Engine实现http.Handler接口的关键方法
func (engine *Engine) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	var middlewares []HandlerFunc
	for _, group := range engine.groups {
		if strings.HasPrefix(req.URL.Path, group.prefix) {
			middlewares = append(middlewares, group.middlewares...)
		}
	}
	c := newContext(w, req)
	c.engine = engine
	c.handlers = middlewares
	engine.router.handle(c)
}
