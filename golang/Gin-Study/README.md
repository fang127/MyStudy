# Gin-Study

这是一个Gin框架的学习项目，包含了Gin框架的基本使用、路由、请求处理、中间件等内容。通过这个项目，你可以快速上手Gin框架，了解其核心功能和最佳实践。

## go原生HTTP包的使用方法：

```go
http.HandleFunc("/index", func(w http.ResponseWriter, r *http.Request) {
    // 1. 获取查询参数
    age := r.URL.Query().Get("age")
    // 2. 获取表单参数
    name := r.FormValue("name")
    // 3. 获取路径参数
    // 4. 响应数据
    w.Write([]byte("Hello, World!"))
})
```

缺点：

- 需要手动解析请求数据，代码冗长。

- 没有内置的路由功能，无法方便地定义不同的路由和处理函数。

- 没有内置的中间件机制，无法方便地处理请求前后的逻辑。

- 响应数据需要手动设置响应头和响应体，代码冗长。

## Gin介绍

Gin是一个用Go语言编写的高性能Web框架，具有以下特点：

- 快速：Gin使用了httprouter作为底层路由引擎，具有非常快的路由匹配速度。

- 简单：Gin提供了简洁的API，易于上手和使用。

- 强大：Gin内置了丰富的功能，如路由分组、参数绑定、JSON序列化、错误处理等，满足各种Web开发需求。

- 中间件支持：Gin支持中间件，可以方便地在请求处理过程中添加各种功能，如日志记录、身份验证、跨域处理等。

- 社区活跃：Gin拥有一个活跃的社区，提供了大量的第三方中间件和插件，可以满足各种特殊需求。

## Gin获取

首先，确保你已经安装了Go环境。然后，你可以使用以下命令来安装Gin框架：

```bash
go get -u github.com/gin-gonic/gin
```

## 基本使用

### 1. 初始化Gin

```go
// 1. 初始化
gin.SetMode(gin.ReleaseMode)
r := gin.Default()
// 2. 挂载路由
r.GET("/index", func(c *gin.Context) {
})
// 3. 绑定端口，开始运行
r.Run(":8080")
```

- 可以使用`gin.SetMode(gin.ReleaseMode)`来设置Gin的运行模式，默认为`debug`模式，适用于开发环境。`release`模式适用于生产环境，可以提高性能。
- `gin.Default()`会返回一个默认的Gin引擎实例，包含了Logger和Recovery中间件。

### 2. 响应JSON数据

```go
c.JSON(200, gin.H{
    "message": "Hello, Gin!",
})
```
`c.JSON`方法用于返回JSON格式的响应，第一个参数是HTTP状态码，第二个参数是要返回的数据，可以使用`gin.H`来创建一个简单的键值对。

### 3. 响应HTML页面

```go
r := gin.Default()
r.LoadHTMLGlob("templates/*")
// r.LoadHTMLFiles("templates/index.html") 该方法加载某个文件
r.GET("", func(c *gin.Context) {
	// 第二个参数是文件名，不一定是html
	// 只写文件名,所以需要给Gin提供文件路径
	c.HTML(http.StatusOK, "index.html", map[string]any{
		"title": "MyBlog",
	})
})
err := r.Run(":8080")
if err != nil {
	log.Fatal(err)
}
```

`r.LoadHTMLGlob("templates/*")`用于加载HTML模板文件，`c.HTML`方法用于返回HTML响应，第一个参数是HTTP状态码，第二个参数是模板文件名，第三个参数是传递给模板的数据。

- 模板内容可以使用`{{.title}}`来访问传递的数据。

### 4. 响应文件

```go
r.GET("", func(c *gin.Context) {
	// 设置响应头，告诉浏览器这是一个文件下载
	c.Header("Content-Type", "application/octet-stream")            // 表示这一个文件流，唤起浏览器的下载功能
	c.Header("Content-Disposition", "attachment; filename=main.go") // 指定下文件的名称
	c.File("main.go")
})
```

`c.File`方法用于返回一个文件作为响应，`Content-Type`设置为`application/octet-stream`可以让浏览器识别为文件下载，`Content-Disposition`设置为`attachment; filename=main.go`可以指定下载文件的名称。

- 你也可以使用`c.FileAttachment("main.go", "main.go")`来简化这个过程。

### 5. 静态文件

```go
r.Static("/static", "./static")
```

`r.Static("/st", "./static")`用于将`./static`目录下的文件映射到`/st`路径下，这样你就可以通过访问`http://localhost:8080/st/filename`来访问静态文件了。

### 6. 路由参数

```go
// 该路由将匹配 /user/john，但不匹配 /user/john/ 或 /user/john/send
router.GET("/user/:name", func(c *gin.Context) {
	name := c.Param("name")
	c.String(http.StatusOK, "Hello %s", name)
})
// 该路由将匹配 /user/john/send 和 /user/john/，但不匹配 /user/john
router.GET("/user/:name/*action", func(c *gin.Context) {
	name := c.Param("name")
	action := c.Param("action")
	message := name + " is " + action
	c.String(http.StatusOK, message)
})
```

-  `:name`匹配单个路径段，例如：`/user/:name` 匹配 `/user/john`，但不匹配 `/user/` 或 `/user`。

-  `*action`匹配任意路径段，例如：`/user/:name/*action` 匹配 `/user/john/send` 和 `/user/john/`，但不匹配 `/user/john`。

### 7. 查询参数

```go
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
```

`c.Query("age")`用于获取查询参数`age`的值，如果没有这个参数则返回空字符串。`c.DefaultQuery("name", "Guest")`用于获取查询参数`name`的值，如果没有这个参数则返回默认值`Guest`。`c.QueryArray("key")`用于获取查询参数`key`的数组形式，如果没有这个参数则返回空数组。

### 8. 表单参数

```go
r.POST("/users", func(c *gin.Context) {
	// 使用 c.GetPostForm 获取 POST 请求中的参数，返回值是参数值和一个布尔值，表示数是否存在
	// 和PostForm的区别在于，如果参数不存在，GetPostForm会返回空字符串和false，PostForm会返回空字符串
	// name, exits:= c.GetPostForm("name")
	// 使用 c.PostForm 获取 POST 请求中的参数
	name := c.PostForm("name")
	message := c.PostForm("message")
	// 使用 c.DefaultPostForm 获取 POST 请求中的参数，如果参数不存在则返回默认值
	age := c.DefaultPostForm("age", "18")
	c.String(http.StatusOK, "name: %s, age: %s, message: %s", name, age,message)
})
```

使用 `c.PostForm()` 和 `c.DefaultPostForm()` 来读取表单提交的值。这些方法适用于 `application/x-www-form-urlencoded` 和 `multipart/form-data` 内容类型——这是浏览器提交表单数据的两种标准方式。

- `c.PostForm("field")` 返回值，如果字段不存在则返回空字符串。

- `c.DefaultPostForm("field", "default")` 返回值，如果字段不存在则返回指定的默认值。

- `c.GetPostForm("field")` 返回值和一个布尔值，表示字段是否存在。

### 9. 加载用户上传的文件

```go
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
```

Gin 使处理 `multipart` 文件上传变得简单直接。框架在 `gin.Context` 上提供了内置方法来接收上传的文件：

- `ctx.FormFile("field")` 用于获取单个文件，返回一个 `*multipart.FileHeader` 对象和一个错误。

- `ctx.MultipartForm()` 用于获取所有上传的文件，返回一个 `*multipart.Form` 对象和一个错误，其中包含了所有文件的详细信息。

- `ctx.SaveUploadedFile(file, destination)` 用于将上传的文件保存到服务器指定的位置。

### 10. 原生内容

```go
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
	ctx.Request.Body = io.NopCloser(bytes.NewReader(byteData)) // 重新构造一新的请求体，使用之前读取到的数据
	name := ctx.PostForm("name")
	fmt.Println(name)
	ctx.String(200, "ok")
})
```

在处理原生内容时，读取请求体数据会导致请求体被关闭，因此后续无法再读取。解决办法是在第一次读取请求体数据之后，重新构造一个新的请求体，使用之前读取到的数据。这样就可以继续读取请求体中的其他参数了。

### 11. binding绑定数据

```go
// 1. 查询参数
r.GET("/index", func(c *gin.Context) {
	type User struct {
		Name string `form:"name"`
		Age  int    `form:"age"`
	}
	var user User
	err := c.ShouldBindQuery(&user)
	if err != nil {
		c.JSON(400, gin.H{"error": err.Error()})
		return
	}
	c.JSON(200, gin.H{"name": user.Name, "age": user.Age})
})
// 2. 路径参数
r.GET("/user/:id/:name", func(c *gin.Context) {
	type User struct {
		ID   string `uri:"id"`
		Name string `uri:"name"`
	}
	var user User
	err := c.ShouldBindUri(&user)
	if err != nil {
		c.JSON(400, gin.H{"error": err.Error()})
		return
	}
	c.JSON(200, gin.H{"id": user.ID, "name": user.Name})
})
// 3. 表单参数
r.POST("/form", func(c *gin.Context) {
	type User struct {
		Name string `form:"name"`
		Age  int    `form:"age"`
	}
	var user User
	// ShouldBind 会根据 HTTP 方法和 Content-Type 请求头自动选择绑定引擎
	// 注意：ShouldBind会根据请求的Content-Type自动选择绑定方式
	// 如果Content-Type是application/x-www-form-urlencoded或multipartform-data，则绑定表单参数
	err := c.ShouldBind(&user)
	if err != nil {
		c.JSON(400, gin.H{"error": err.Error()})
		return
	}
	c.JSON(200, gin.H{"name": user.Name, "age": user.Age})
})
```

Gin 提供了强大的数据绑定功能，可以将请求中的数据自动绑定到结构体中。根据不同的请求类型和参数来源，Gin 提供了不同的绑定方法：

- 类型：Must bind

    - `BindQuery` 用于绑定查询参数，如果绑定失败会返回错误。

    - `BindUri` 用于绑定路径参数，如果绑定失败会返回错误。

    - `Bind` 会根据 HTTP 方法和 Content-Type 请求头自动选择绑定引擎，适用于表单参数，如果绑定失败会返回错误。

    - `BindJSON` 用于绑定 JSON 数据，如果绑定失败会返回错误。

    - `BindXML` 用于绑定 XML 数据，如果绑定失败会返回错误。

    - 行为：这些方法底层使用 `MustBindWith`。如果存在绑定错误，请求将使用 `c.AbortWithError(400, err).SetType(ErrorTypeBind)` 中止。这会将响应状态码设置为 400，并将 `Content-Type`头设置为 `text/plain; charset=utf-8`。注意，如果你在此之后尝试设置响应码，将会出现警告 `[GIN-debug] [WARNING] Headers were already written. Wanted to override status code 400 with 422`。如果你希望更好地控制行为，请考虑使用 `ShouldBind` 等效方法。

- 类型：Should bind

    - `ShouldBindQuery` 用于绑定查询参数。

    - `ShouldBindUri` 用于绑定路径参数。

    - `ShouldBind` 会根据 HTTP 方法和 Content-Type 请求头自动选择绑定引擎，适用于表单参数。

    - `ShouldBindJSON` 用于绑定 JSON 数据。

    - `ShouldBindXML` 用于绑定 XML 数据。

    - 行为：这些方法底层使用 `ShouldBindWith`。如果存在绑定错误，错误会被返回，由开发者负责适当地处理请求和错误。

注意：需要在结构体字段上使用标签来指定绑定的参数名称，例如 `form:"name"` 表示绑定查询参数或表单参数中的 `name` 字段，`uri:"id"` 表示绑定路径参数中的 `id` 字段，这样 Gin 才能正确地将请求中的数据绑定到结构体中。

你还可以指定特定字段为必填。如果一个字段标记了 `binding:"required"` 并且在绑定时值为空，将返回错误。

如果结构体的某个字段本身也是结构体（嵌套结构体），该结构体的字段也需要标记 `binding:"required"` 才能正确验证。

```go
// Binding from JSON
type Login struct {
  User     string `form:"user" json:"user" xml:"user"  binding:"required"`
  Password string `form:"password" json:"password" xml:"password" binding:"required"`
}
```

