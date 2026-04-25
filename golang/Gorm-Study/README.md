# Gorm-Study

这是一个使用Gorm进行数据库操作的学习项目。Gorm是一个流行的Go语言ORM库，提供了丰富的功能和简洁的API，使得数据库操作更加方便和高效。

## 什么是Gorm？

ORM是对象关系映射的缩写，它是一种将数据库表映射为编程语言中的对象的技术。简单说就是使用一个结构体或类来表示数据库中的一张表，类的属性对应表中的字段。Gorm就是这样一个ORM库，它允许开发者使用Go语言的结构体来定义数据库模型，并通过方法调用来执行CRUD（创建、读取、更新、删除）操作。

Gorm是一个Go语言的ORM（对象关系映射）库，它提供了一个简单而强大的接口来操作数据库。Gorm支持多种数据库，包括MySQL、PostgreSQL、SQLite等，允许开发者使用Go语言的结构体来定义数据库模型，并通过方法调用来执行CRUD（创建、读取、更新、删除）操作。

1. 和自动生成SQL语句相比，手动编写SQL语句的缺点是非常明显的，主要为：

- 对象的属性名和数据表的字段名往往不一致，因此编写SQL语句时要注意字段名和属性名的对应关系。

- 此外，当SQL语句出错，错误信息往往是数据库返回的，可能不够直观，难以定位问题。

- 不同数据库的SQL语法可能存在差异，手动编写SQL语句时需要考虑兼容性问题。

- SQL注入攻击的风险较高，如果不小心处理用户输入，可能会导致安全漏洞。

2. ORM的缺点：

- ORM增加了学习曲线，开发者需要学习和理解ORM的使用方法和原理。

- ORM可能会引入性能开销，尤其是在处理复杂查询或大量数据时，可能不如手动编写SQL语句高效。

- 对于复杂的查询，ORM可能无法提供足够的灵活性，开发者可能需要编写原生SQL来满足需求。

- 生成SQL语句的过程是自动的，不能人工干预，这使得开发者无法定制一些特殊的SQL语句。

## Gorm官方

https://gorm.io/zh_CN/docs/index.html

## 传统的数据库操作方式



## 入门教程

### 1. 传统数据库连接

```go
package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/go-sql-driver/mysql" // 导入MySQL驱动，使用匿名导入方式，因为我们只需要注册驱动，不需要直接使用包中的函数。
)

// 传统方法操作数据库，使用sql包，手动编写SQL语句，执行查询和更新操作。
func main() {
	// dsn是数据源名称，包含了连接数据库所需的信息，如用户名、密码、主机地址、端口号和数据库名称。
	// 格式为：username:password@protocol(address)/dbname?param=value
	dsn := "harry:123456@tcp(127.0.0.1:3306)/gorm_study"
	db, err := sql.Open("mysql", dsn)
	// 这里的错误不会管密码错误、数据库不存在等连接错误，因为sql.Open只是验证了参数的格式，并没有真正建立连接。
	if err != nil {
		log.Fatalln(err)
	}
	err = db.Ping() // 通过Ping方法来验证数据库连接是否成功，如果连接失败会返回错误。
	if err != nil {
		log.Fatalln("数据库连接失败", err)
	}
	fmt.Println(db)
}
```

- sql.Open函数用于打开一个数据库连接，参数包括数据库驱动名称（如"mysql"）和数据源名称（DSN）。DSN包含了连接数据库所需的信息，如用户名、密码、主机地址、端口号和数据库名称。

- sql.Open函数不会立即建立连接，而是返回一个数据库对象。要验证连接是否成功，需要调用db.Ping()方法，如果连接失败会返回错误。

- 注意，连接对应的引擎，必须导入相应的数据库驱动包，如MySQL需要导入"github.com/go-sql-driver/mysql"。这里使用匿名导入方式，因为我们只需要注册驱动，不需要直接使用包中的函数。

- 这种传统的数据库操作方式需要手动编写SQL语句，处理查询结果，容易出错且不够高效。相比之下，使用ORM库如Gorm可以简化数据库操作，提高开发效率。

```go
// 2. 执行查询操作
rows, err := db.Query("SELECT id, name FROM users")
if err != nil {
	log.Fatalln("查询失败", err)
}
defer rows.Close() // 确保在函数结束时关闭rows，释放数据库资源。
for rows.Next() { // 迭代查询结果
	var id int
	var name string
	err := rows.Scan(&id, &name) // 将查询结果扫描到变量中
	if err != nil {
		log.Fatalln("扫描失败", err)
	}
	fmt.Printf("ID: %d, Name: %s\n", id, name) // 打印查询结果
}
// 3. 执行更新操作
result, err := db.Exec("UPDATE users SET name = ? WHERE id = ?", "NewName", 1)
if err != nil {
	log.Fatalln("更新失败", err)
}
rowsAffected, err := result.RowsAffected() // 获取受影响的行数
if err != nil {
	log.Fatalln("获取受影响行数失败", err)
}
fmt.Printf("更新成功，受影响的行数: %d\n", rowsAffected)
```

- db.Query方法用于执行查询操作，返回一个rows对象，可以通过迭代rows来获取查询结果。需要注意在使用完rows后调用rows.Close()来释放数据库资源。

- db.Exec方法用于执行增删改操作，返回一个result对象，可以通过result.RowsAffected()方法获取受影响的行数。

> 这种传统的数据库操作方式需要手动编写SQL语句，处理查询结果，容易出错且不够高效。相比之下，使用ORM库如Gorm可以简化数据库操作，直接将数据库表映射为Go语言的结构体，提供了更高层次的抽象，使得开发者可以更专注于业务逻辑，而不需要关心底层的SQL细节。

### 2. Gorm小试牛刀

```go
// User 定义了一个用户模型，包含ID、Name、Age和Email字段
type User struct {
	// gorm.Model
	ID    int
	Name  string
	Age   int
	Email string
}

func main() {
	dsn := "harry:123456@tcp(127.0.0.1:3306)/gorm_study"
	// 1. 连接数据库
	db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Fatal(err)
		return
	}
	fmt.Println(db)

	var users []User
	db.Find(&users)
	fmt.Println(users)
}
```

- 首先定义了一个User结构体，表示数据库中的用户表。结构体的字段对应数据库表的列，Gorm会根据结构体自动映射到数据库表。

- 使用gorm.Open函数连接数据库，参数包括数据库驱动和数据源名称（DSN）。Gorm会自动处理连接细节，并返回一个数据库对象。

- 使用db.Find(&users)方法查询所有用户记录，并将结果存储在users切片中。Gorm会自动生成SQL查询语句并执行，简化了数据库操作。

- Gorm怎么知道User结构体对应数据库中的哪个表？默认情况下，Gorm会将结构体名称转换为蛇形命名（snake_case）并加上复数形式来推断表名。例如，User结构体会被映射到users表。如果需要自定义表名，可以在结构体中使用TableName方法来指定：

```go
func (User) TableName() string {
	return "my_users" // 指定表名为my_users
}
```

### 3. 单表模型和单表操作

1. 表结构

在gorm中，使用结构体来表示一张表，结构体的字段对应表中的列。可以通过在结构体字段上添加标签来指定列名、数据类型、约束等信息。例如：

```go
type User struct {
	ID    int    `gorm:"primaryKey;autoIncrement"` // 主键，自动递增
	Name  string `gorm:"size:255;not null"`        // 字符串，最大长度255，不能为空
	Age   int    `gorm:"not null"`                 // 整数，不能为空
	Email string `gorm:"size:255;unique"`          // 字符串，最大长度255，唯一
}
```

通常情况下，会使用gorm.Model作为基础结构体，它包含了ID、CreatedAt、UpdatedAt和DeletedAt字段，提供了常用的模型功能，如自动生成ID和时间戳等。例如：

```go
type User struct {
	/*
		其他字段...
	*/
	gorm.Model // 包含ID、CreatedAt、UpdatedAt和DeletedAt字段
}
```

2. 表迁移

Gorm提供了自动迁移功能，可以根据结构体定义自动创建或更新数据库表结构。使用db.AutoMigrate(&User{})方法可以自动迁移User模型到数据库中。例如：

```go
db.AutoMigrate(&User{})
```

- 这会检查数据库中是否存在与User模型对应的表，如果不存在则创建表，如果存在则根据模型定义更新表结构（如添加新列），但是**不会删除或更新已有的列**。

- 需要注意的是，自动迁移功能虽然方便，但在生产环境中使用时要谨慎，因为它可能会对数据库结构进行修改，导致数据丢失或应用异常。建议在开发阶段使用自动迁移，在生产环境中使用手动迁移工具来管理数据库结构变更。

3. 插入

使用db.Create(&user)方法可以插入一条记录到数据库的users表中。例如：

```go
user := User{Name: "Alice", Age: 30, Email: "xxxx@163.com"}
db.Create(&user)
```

- 这会将user对象插入到数据库中，并自动生成ID和时间戳等字段。插入成功后，user对象的ID字段会被更新为数据库生成的ID值。

4. 批量插入

使用db.Create(&users)方法可以批量插入多条记录到数据库中。例如：

```go
users := []User{
	{Name: "Bob", Age: 25, Email: "xxxx"},
	{Name: "Charlie", Age: 35, Email: "xxxx"},
}
db.Create(&users)
```

- 这会将users切片中的所有User对象插入到数据库中，并自动生成ID和时间戳等字段。批量插入可以提高性能，减少数据库连接的开销。

5. hook函数

Gorm提供了hook函数，可以在模型的生命周期中执行特定的操作。例如，可以定义BeforeCreate、AfterCreate、BeforeUpdate、AfterUpdate等函数来在创建或更新记录之前或之后执行一些逻辑。例如：

```go
func (u *User) BeforeCreate(tx *gorm.DB) (err error) {
	// 可以在内部进行一些数据验证或自动填充字段的操作
	// u.Name = fmt.Sprintf("User_%d", time.Now().Unix()) // 生成一个唯一的用户名
	if u.Name == "" {
		return errors.New("Name cannot be empty")
	}
	return nil
}
```

- 这个BeforeCreate函数会在创建User记录之前被调用，如果返回一个错误，创建操作将会被取消，并且错误信息会被返回给调用者。可以在hook函数中执行各种逻辑，如数据验证、设置默认值、记录日志等，以确保数据的完整性和一致性。

6. 查询

```go
var user User
db.First(&user, 1) // 根据主键查询ID为1的记录
db.First(&user, "name = ?", "Alice") // 根据条件查询name为Alice的记录

var users []User
db.Find(&users) // 查询所有记录
db.Find(&users, "age > ?", 30) // 查询年龄大于30的记录
db.Where("age > ?", 30).Find(&users) // 查询年龄大于30的记录
db.Where("name IN ?", []string{"Alice", "Bob"}).Find(&users) // 查询name在Alice和Bob中的记录
db.Where("name LIKE ?", "%li%").Find(&users) // 查询name包含li的记录


db.Take(&user) // 查询一条记录，随机返回一条
db.Last(&user) // 查询最后一条记录
db.Take(&user, "age > ?", 30) // 查询年龄大于30的一条记录
db.Last(&user, "age > ?", 30) // 查询年龄大于30的最后一条记录

db.Debug().Where("age > ?", 30).Find(&users) // 输出生成的SQL语句，调试查询

db.Select("name, age").Where("age > ?", 30).Find(&users) // 只查询name和age字段

db.Order("age desc").Find(&users) // 按年龄降序查询

db.Limit(10).Find(&users) // 查询前10条记录

db.Offset(10).Find(&users) // 跳过前10条记录查询

db.Where("age > ?", 30).Count(&count) // 统计年龄大于30的记录数

user = &models.UserModel{}
err = global.DB.Take(&user, 100).Error
// gorm提供了一些预定义的错误变量，如ErrRecordNotFound、ErrInvalidData等，可以通过比较错误变量来判断错误类型。
if err == gorm.ErrRecordNotFound {
	fmt.Println("Record not found")
}

```

- Gorm提供了丰富的查询方法，可以根据主键、条件、范围等多种方式查询记录。可以使用First、Find、Take、Last等方法来获取单条或多条记录，并且支持链式调用来构建复杂的查询条件。

- Find不会返回错误，如果没有找到记录，返回的切片会是空的。而First、Take、Last等方法如果没有找到记录，会返回ErrRecordNotFound错误。需要根据具体需求选择合适的查询方法，并处理可能出现的错误情况。

- 通过使用Debug()方法，可以输出Gorm生成的SQL语句，方便调试查询逻辑。

- 还可以使用Select方法来指定查询的字段，Order方法来指定排序方式，Limit和Offset方法来控制查询结果的数量和偏移量，以及Count方法来统计记录数等。

7. 更新