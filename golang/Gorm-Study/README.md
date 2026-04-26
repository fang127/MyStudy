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

#### 3.1 表结构

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

#### 3.2 表迁移

Gorm提供了自动迁移功能，可以根据结构体定义自动创建或更新数据库表结构。使用db.AutoMigrate(&User{})方法可以自动迁移User模型到数据库中。例如：

```go
db.AutoMigrate(&User{})
```

- 这会检查数据库中是否存在与User模型对应的表，如果不存在则创建表，如果存在则根据模型定义更新表结构（如添加新列），但是**不会删除或更新已有的列**。

- 需要注意的是，自动迁移功能虽然方便，但在生产环境中使用时要谨慎，因为它可能会对数据库结构进行修改，导致数据丢失或应用异常。建议在开发阶段使用自动迁移，在生产环境中使用手动迁移工具来管理数据库结构变更。

#### 3.3 插入

使用db.Create(&user)方法可以插入一条记录到数据库的users表中。例如：

```go
user := User{Name: "Alice", Age: 30, Email: "xxxx@163.com"}
db.Create(&user)
```

- 这会将user对象插入到数据库中，并自动生成ID和时间戳等字段。插入成功后，user对象的ID字段会被更新为数据库生成的ID值。

#### 3.4 批量插入

使用db.Create(&users)方法可以批量插入多条记录到数据库中。例如：

```go
users := []User{
	{Name: "Bob", Age: 25, Email: "xxxx"},
	{Name: "Charlie", Age: 35, Email: "xxxx"},
}
db.Create(&users)
```

- 这会将users切片中的所有User对象插入到数据库中，并自动生成ID和时间戳等字段。批量插入可以提高性能，减少数据库连接的开销。

#### 3.5 hook函数

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

#### 3.6 查询

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
err = db.Take(&user, 100).Error
// gorm提供了一些预定义的错误变量，如ErrRecordNotFound、ErrInvalidData等，可以通过比较错误变量来判断错误类型。
if err == gorm.ErrRecordNotFound {
	fmt.Println("Record not found")
}

```

- Gorm提供了丰富的查询方法，可以根据主键、条件、范围等多种方式查询记录。可以使用First、Find、Take、Last等方法来获取单条或多条记录，并且支持链式调用来构建复杂的查询条件。

- Find不会返回错误，如果没有找到记录，返回的切片会是空的。而First、Take、Last等方法如果没有找到记录，会返回ErrRecordNotFound错误。需要根据具体需求选择合适的查询方法，并处理可能出现的错误情况。

- 通过使用Debug()方法，可以输出Gorm生成的SQL语句，方便调试查询逻辑。

- 还可以使用Select方法来指定查询的字段，Order方法来指定排序方式，Limit和Offset方法来控制查询结果的数量和偏移量，以及Count方法来统计记录数等。

#### 3.7 更新

Save 会保存所有字段（记录存在），或者插入（记录不存在）；Update 用于更新单列；Updates 支持 struct 或 map 多列更新，struct 默认只更新非零值字段；UpdateColumn/UpdateColumns 会跳过 Hooks 和更新时间追踪。

```go
var user User
db.First(&user, 1)

user.Name = "Tom"
user.Age = 0

db.Save(&user) // 更新所有字段，Age会被更新为0
```

- 更新所有字段，包括零值，比如 `0、false、""`

```go
// 危险：Email、Age、Active 等未赋值字段可能被覆盖为零值
user := User{
    ID:   1,
    Name: "Tom",
}

db.Save(&user)
```

| 行为               | 说明                                                         |
| ---------------- | ---------------------------------------------------------- |
| 更新字段             | **所有字段**，包括零值，比如 `0`、`false`、`""`                          |
| 是否更新 `UpdatedAt` | 是                                                          |
| 是否触发 Hook        | 是，比如 `BeforeSave`、`BeforeUpdate`、`AfterUpdate`、`AfterSave` |
| 主键存在             | 执行 `UPDATE`                                                |
| 主键不存在            | 执行 `INSERT`                                                |
| `UPDATE` 影响行数为 0 | 会 fallback 到 `Create`，也就是可能插入                              |

- 不要把 Save 和 Model 连用，这是 undefined behavior；并且 Generics API 中已经故意移除了 Save，建议用 Create 或 Updates。

```go
db.Model(&User{}).Where("id = ?", 1).Update("name", "Tom")
```

对应的 SQL 语句：

```sql
UPDATE users
SET name='Tom', updated_at='...'
WHERE id = 1;
```

| 行为               | 说明                                |
| ---------------- | --------------------------------- |
| 更新字段             | 单字段                               |
| 是否更新 `UpdatedAt` | 是                                 |
| 是否触发 Hook        | 是                                 |
| 是否需要条件           | 需要 `WHERE` 或通过 `Model(&user)` 带主键 |
| 零值能否更新           | 可以，比如 `Update("age", 0)`          |

如果 Model 传入的对象有主键，GORM 会自动用主键作为条件：

```go
user := User{ID: 2}
db.Model(&user).Update("name", "Tom")
```

对应的 SQL 语句：

```sql
UPDATE users
SET name='Tom', updated_at='...'
WHERE id = 2;
```

- 如果没有条件，Update 会触发 ErrMissingWhereClause，防止全表更新。

```go
db.Model(&User{}).Where("id = ?", 1).Updates(User{
    Name:   "Tom",
    Age:    0,
    Active: false,
})
```

对应的 SQL 语句：

```sql
UPDATE users
SET name='Tom', updated_at='...'
WHERE id = 1;
```

- Updates 使用 struct 时，默认只更新非零值字段。所以这里 Age: 0 和 Active: false 不会更新。

```go
db.Model(&User{}).Where("id = ?", 1).Updates(map[string]interface{}{
    "name":   "Tom",
    "age":    0,
    "active": false,
})
```

对应的 SQL 语句：

```sql
UPDATE users
SET name='Tom', age=0, active=false, updated_at='...'
WHERE id = 1;
```

- 使用 map 进行 Updates 时，所有字段都会更新，包括零值。

- 但是使用Select指定字段，也可以强制更新struct零值，比如：

```go
db.Model(&User{}).Where("id = ?", 1).Select("Name", "Age", "Active").Updates(User{
    Name:   "Tom",
    Age:    0,
    Active: false,
})
```

```go
db.Model(&User{}).Where("id = ?", 1).UpdateColumn("score", gorm.Expr("score + ?", 10))
```

对应的 SQL 语句：

```sql
UPDATE users
SET score = score + 10
WHERE id = 1;
```

| 行为               | 说明                         |
| ---------------- | -------------------------- |
| 更新字段             | 单字段                        |
| 是否更新 `UpdatedAt` | **否**                      |
| 是否触发 Hook        | **否**                      |
| 常见用途             | 计数器、库存扣减、状态位快速更新、避免业务 Hook |

- 如果想跳过 Hooks 并且不追踪更新时间，可以用 UpdateColumn / UpdateColumns，它们分别类似于 Update / Updates。

```go
db.Model(&User{}).Where("id = ?", 1).UpdateColumns(map[string]interface{}{
    "score": 100,
    "level": 2,
})
```

**总结**

| 方法                | 更新范围 |                            零值字段 | Hook | `UpdatedAt` | 是否可能插入 | 典型用途              |
| ----------------- | ---: | ------------------------------: | ---: | ----------: | -----: | ----------------- |
| `Save`            |  全字段 |                              更新 |   触发 |          更新 |     可能 | 全量保存，不推荐做普通局部更新   |
| `Update`          |  单字段 |                              更新 |   触发 |          更新 |     不会 | 更新一个字段            |
| `Updates(struct)` |  多字段 |                            默认忽略 |   触发 |          更新 |     不会 | DTO 非零值局部更新       |
| `Updates(map)`    |  多字段 |                              更新 |   触发 |          更新 |     不会 | 推荐的多字段更新，尤其包含零值   |
| `UpdateColumn`    |  单字段 |                              更新 |  不触发 |         不更新 |     不会 | 计数器、库存、跳过 Hook    |
| `UpdateColumns`   |  多字段 | `map` 可更新零值；`struct` 默认仍需注意零值选择 |  不触发 |         不更新 |     不会 | 批量字段更新但跳过 Hook/时间 |


#### 3.8 删除

```go
var user models.UserModel
user.ID = 3
db.Delete(&user) // 根据主键删除

db.Where("age > ?", 30).Delete(&models.UserModel{}) // 根据条件删除
db.Delete(&models.UserModel{}, 3) // 直接根据主键删除
```

- 删除操作会根据主键或条件删除记录，如果模型定义了`DeletedAt`字段，Gorm会执行软删除，即将`DeletedAt`字段设置为当前时间，而不是直接从数据库中删除记录。可以通过`Unscoped()`方法来执行硬删除，直接从数据库中删除记录。软删除的记录在查询时会被自动过滤掉，除非使用`Unscoped()`方法来包含软删除的记录。

```go
db.Unscoped().Where("age > ?", 30).Find(&users) // 查询所有记录，包括软删除的记录

db.Unscoped().Delete(&user) // 硬删除，直接从数据库中删除记录
```

- 需要注意的是，删除操作会触发相应的Hook函数，如`BeforeDelete`、`AfterDelete`等，可以在这些函数中执行一些逻辑，如记录日志、清理相关数据等，以确保数据的一致性和完整性。

- 当试图执行不带任何条件的`Delete`操作时，Gorm会返回`ErrMissingWhereClause`错误，以防止意外删除整个表的数据。

#### 3.9 高级查询

**1. 常用的查询方法**

```go
db.Where("age > ?", 30).Or("name = ?", "Alice").Find(&users) // 查询年龄大于30或名字为Alice的记录
query := db.Where("age > ?", 30).Or("name = ?", "Alice") // 构建查询对象
query.Find(&users) // 执行查询
db.Not("name = ?", "Bob").Find(&users) // 查询名字不为Bob的记录
db.Or("name = ?", "Charlie").Or("name = ?", "Dave").Find(&users) // 查询名字为Charlie或Dave的记录
db.Where("age > ?", 30).Where("name != ?", "Bob").Find(&users) // 查询年龄大于30且名字不为Bob的记录
db.And("age > ?", 30).Or("name = ?", "Alice").Find(&users) // 查询年龄大于30且名字不为Bob的记录，或者名字为Alice的记录
db.Order("age desc").Limit(10).Find(&users) // 查询年龄大于30的记录，按年龄降序排序，限制返回10条记录

// 分页查询
page := 2
pageSize := 10
db.Where("age > ?", 30).Offset((page - 1) * pageSize).Limit(pageSize).Find(&users) // 查询年龄大于30的记录，分页返回，每页10条记录，返回第2页的数据

db.Where("age > ?", 30).Count(&count) // 统计年龄大于30的记录数
db.Group("age").Having("count(*) > ?", 1).Find(&users) // 按年龄分组，统计每个年龄的记录数，筛选出记录数大于1的年龄组
```
- Gorm提供了丰富的查询方法，可以通过链式调用来构建复杂的查询条件，如`Where`、`Or`、`Not`、`And`等方法来组合查询条件，`Order`方法来指定排序方式，`Limit`和`Offset`方法来控制查询结果的数量和偏移量，以及`Count`方法来统计记录数等。

- 对于注入攻击，Gorm会自动对查询参数进行转义和参数化处理，防止SQL注入攻击的发生。开发者只需要使用占位符（如`?`）来传递查询参数，Gorm会自动处理参数的安全性。

**2. Scan方法和Pluck方法**

```go
type UserDTO struct {
    ID   uint
    Name string
    Age  int
}

var users []UserDTO

err := db.Model(&User{}).
    Select("id, name, age").
    Where("age > ?", 18).
    Scan(&users).Error
```

- `Scan`方法可以将查询结果扫描到任意类型的变量中，不局限于模型结构体。可以使用Scan方法来执行一些特殊的查询，如聚合函数、连接查询等，并将结果存储在自定义的结构体或基本类型变量中。

```go
var names []string
db.Model(&User{}).Pluck("name").Find(&names) // 查询所有用户的名字，结果存储在names切片中
```

- `Scan` 和 `Pluck` 都是 把查询结果扫描到变量里，但定位不同：

	- `Pluck` 只适合查 单列，结果放进 `slice`；`Scan` 更通用，可以查 单列、多列、聚合结果、自定义结构体、原生 SQL 结果。

	- 和 `Find` 不同，`Find` 是 GORM 的模型查询方法，会按照 Model 规则处理，比如表名、字段映射、关联、软删除条件等。

- 要注意，`Scan`方法不会自动处理模型的字段映射和关联关系，因此在使用Scan时需要确保查询结果的列名与目标结构体的字段名一致，或者使用别名来匹配字段名。

| 对比项        | `Pluck`         | `Scan`                  |
| ---------- | --------------- | ----------------------- |
| 查询列数       | **只能单列**        | 单列、多列都可以                |
| 接收目标       | 通常是 `[]基础类型`    | 结构体、结构体切片、基础变量都可以       |
| 是否适合 DTO   | 不适合             | 适合                      |
| 是否适合聚合查询   | 只适合单列聚合         | 适合复杂聚合                  |
| 是否适合原生 SQL | 不常用             | 很适合                     |
| 典型场景       | 查 ID 列表、name 列表 | 查自定义结果、报表、JOIN、GROUP BY |

对于`Scan`和`Pluck`方法，去重复合查询结果时，可以使用`DISTINCT`关键字来实现。例如：

```go
var names []string
db.Model(&User{}).Select("DISTINCT name").Scan(&names) // 查询所有用户的唯一名字，结果存储在names切片中
db.Model(&User{}).Distinct("name").Scan(&names) // 等价于上面，查询所有用户的唯一名字，结果存储在names切片中
```

**3. Scopes方法**

```go
db.Scopes(func(db *gorm.DB) *gorm.DB {
	return db.Where("age > ?", age)
}).Find(&users)
```

- `Scopes`方法可以定义一个函数，接受一个`*gorm.DB`对象作为参数，并返回一个修改后的`*gorm.DB`对象。这样可以将一些常用的查询条件封装成函数，方便在多个地方复用。例如，可以定义一个`AgeGreaterThan`函数来封装年龄大于某个值的查询条件：

```go
func AgeGreaterThan(age int) func(db *gorm.DB) *gorm.DB {
	return func(db *gorm.DB) *gorm.DB {
		return db.Where("age > ?", age)
	}
}

// 使用示例
db.Scopes(AgeGreaterThan(30)).Find(&users) // 查询年龄大于30的用户
```

**4. 钩子函数**

有如下几种主要钩子函数：

- `BeforeSave`：在保存记录之前调用，可以用于数据验证、自动填充字段等操作。

- `AfterSave`：在保存记录之后调用，可以用于记录日志、发送通知等操作。

- `BeforeCreate`：在创建记录之前调用，可以用于数据验证、自动填充字段等操作。

- `AfterCreate`：在创建记录之后调用，可以用于记录日志、发送通知等操作。

- `BeforeUpdate`：在更新记录之前调用，可以用于数据验证、自动填充字段等操作。

- `AfterUpdate`：在更新记录之后调用，可以用于记录日志、发送通知等操作。

- `BeforeDelete`：在删除记录之前调用，可以用于记录日志、清理相关数据等操作。

- `AfterDelete`：在删除记录之后调用，可以用于记录日志、清理相关数据等操作。

- `AfterFind`：在查询记录之后调用，可以用于数据处理、格式化等操作。

- `AfterScan`：在扫描记录之后调用，可以用于数据处理、格式化等操作。

### 4. 原生SQL

使用Rows方法执行原生SQL查询：

```go
rows, err := db.Raw("SELECT id, name FROM users WHERE age > ?", 30).Rows()
if err != nil {
	log.Fatalln("查询失败", err)
}
defer rows.Close()
for rows.Next() {
	var id int
	var name string
	err := rows.Scan(&id, &name)
	if err != nil {
		log.Fatalln("扫描失败", err)
	}
	fmt.Printf("ID: %d, Name: %s\n", id, name)
}
```

- 使用`db.Raw()`方法可以执行原生SQL查询，返回一个`*sql.Rows`对象，可以通过迭代`rows`来获取查询结果。需要注意在使用完`rows`后调用`rows.Close()`来释放数据库资源。

使用Exec方法执行原生SQL更新：

```go
result := db.Exec("UPDATE users SET name = ? WHERE id = ?", "NewName", 1)
if result.Error != nil {
	log.Fatalln("更新失败", result.Error)
}
rowsAffected := result.RowsAffected
fmt.Printf("更新成功，受影响的行数: %d\n", rowsAffected)
```

- 使用`db.Exec()`方法可以执行原生SQL更新，返回一个`*gorm.DB`对象，可以通过`result.Error`来检查是否执行成功，通过`result.RowsAffected`来获取受影响的行数。

### 6. 迁移

gorm提供了自动迁移功能，可以根据结构体定义自动创建或更新数据库表结构。使用`db.AutoMigrate(&User{})`方法可以自动迁移User模型到数据库中。例如：

```go
db.AutoMigrate(&User{})
```

- AutoMigrate 会创建表、缺失的外键、约束、列和索引。 出于保护您数据的目的，它不会删除未使用的列

此外，Gorm还提供了更细粒度的迁移方法，提供了`Migrator`接口，可以执行更复杂的迁移操作，如添加列、删除列、修改列类型等。例如：

```go
db.Migrator().AddColumn(&User{}, "Email") // 添加Email列
db.Migrator().DropColumn(&User{}, "Age") // 删除Age列
db.Migrator().AlterColumn(&User{}, "Name") // 修改Name列的类型
db.Migrator().RenameColumn(&User{}, "Name", "FullName") // 重命名Name列为FullName
db.Migrator().CreateIndex(&User{}, "Name") // 创建Name列的索引
db.Migrator().DropIndex(&User{}, "Name") // 删除Name列的索
db.Migrator().HasTable(&User{}) // 检查User表是否存在
db.Migrator().HasColumn(&User{}, "Email") // 检查User表是否有Email列
db.Migrator().HasIndex(&User{}, "Name") // 检查User表是否有Name列的索引
// 迁移工具还提供了更多方法，可以根据需要进行使用。
```

- 需要注意的是，自动迁移功能虽然方便，但在生产环境中使用时要谨慎，因为它可能会对数据库结构进行修改，导致数据丢失或应用异常。建议在开发阶段使用自动迁移，在生产环境中使用手动迁移工具来管理数据库结构变更。

### 7. 一对一关系 - 表设计

```go
type UserModel struct {
	ID        int
	Name      string         		 `gorm:"not null;unique"` // 将Name字段设置为主键
	Age       int            		 `gorm:"default:18"`      // 设置Age字段的默认值为18
	UserDetailModel *UserDetailModel `gorm:"foreignKey:UserID"` // 定义一对一关系，指定外键为UserID
	CreatedAt time.Time      // 创建时间，gorm会自动设置这个字段的值为当前时间
	DeletedAt gorm.DeletedAt // 软删除字段，gorm会自动处理这个字段的值
}

type UserDetailModel struct {
	ID       int
	UserID   int	   `gorm:"unique"` // 将UserID字段设置为唯一索引，这样才形成唯一的外键关系
	UserModel *UserModel `gorm:"foreignKey:UserID"`
	Email    string    `gorm:"size:32"`
}
```
- 如果UserModel的ID字段是主键，那么在UserDetailModel中可以直接使用UserID作为外键，无需指定references。

- 要形成一对一关系，需要在UserDetailModel中添加一个外键字段UserID，并使用`gorm:"foreignKey:UserID"`标签来指定这个字段是外键，关联到UserModel的ID字段。

- 如果UserModel的ID字段不是主键，而是Name字段作为主键，那么在UserDetailModel中需要将外键字段改为UserName，并使用`gorm:"foreignKey:UserName;references:Name"`标签来指定外键关联到UserModel的Name字段。例如：

```go
type UserDetailModel struct {
	ID       int
	UserName string    `gorm:"unique"` // 将UserName字段设置为唯一索引，这样才形成唯一的外键关系
	UserModel *UserModel `gorm:"foreignKey:UserName;references:Name"`
	Email    string    `gorm:"size:32"`
}
```
- 在UserModel的主键不是Name字段情况，可以使用`gorm:"foreignKey:UserName;references:Name"`标签来指定外键关联到UserModel的Name字段，这样就形成了基于非主键字段的外键关系。

- 注意，这样的外键是实体级的外键关系，Gorm会根据这个关系*自动处理关联查询、更新和删除*等操作，确保数据的一致性和完整性，因此会有性能开销，一般项目中不建议使用实体级外键关系

**非实体外键**

一般情况都采用非实体外键关系，可以在db连接时指定配置项`DisableForeignKeyConstraintWhenMigrating: true`来禁用自动创建外键约束，这样就不会在数据库层面创建外键约束，但是为了维护数据的一致性和完整性，需要我们在应用层面手动处理关联查询、更新和删除等操作，确保数据的一致性和完整性。

```go
db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{
	DisableForeignKeyConstraintWhenMigrating: true, // 禁用自动创建外键约束
})
```


**插入**

```go
// 一对一关系插入数据
func insert() {
	// 1. 创建用户并创建用户详情
	err := db.Create(&models.UserModel{
		Name: "Helen",
		Age:  29,
		UserDetailModel: &models.UserDetailModel{
			Email: "helen@example.com",
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 2. 创建用户详情并创建用户
	err := db.Create(&models.UserDetailModel{
		Email: "ann@example.com",
		UserModel: &models.UserModel{
			Name: "Ann",
			Age:  27,
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 3. 创建用户并关联已有的用户详情
	err := db.Create(&models.UserModel{
		Name: "Frank",
		Age:  32,
		UserDetailModel: &models.UserDetailModel{
			ID: 1, // 假设已有的用户详情ID为1
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 4. 创建用户详情并关联已有的用户
	err := db.Create(&models.UserDetailModel{
		Email: "ana@example.com",
		UserModel: &models.UserModel{
			ID: 1, // 假设已有的用户ID为1
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
}
```

- 在插入数据时，可以通过在创建用户时同时创建用户详情，或者在创建用户详情时同时创建用户来实现一对一关系的插入。也可以先创建一个用户或用户详情，然后再关联已有的记录来实现插入。

**查询**

```go
func query() {
	var id = 5
	var detail models.UserDetailModel
	// 1. 通过用户详情查询关联的用户信息
	// 预加载用户详情关联的用户信息
	// Preload的第一个参数是关联字段的名称，这里是UserModel
	// Preload会在查询UserDetailModel时同时查询关联的UserModel，并将结果填充到UserModel字段中
	db.Preload("UserModel").Take(&detail, "user_id = ?", id)
	fmt.Printf("用户详情: %+v\n", detail)
	fmt.Printf("关联的用户: %+v\n", detail.UserModel)

	// 2. 通过用户查询关联的用户详情信息
	var user models.UserModel
	db.Preload("UserDetailModel").Take(&user, "id = ?", id)
	fmt.Printf("用户: %+v\n", user)
	fmt.Printf("关联的用户详情: %+v\n", user.UserDetailModel)
}
```

- 在查询数据时，可以使用`Preload`方法来预加载关联的记录，这样在查询用户详情时可以同时查询关联的用户信息，或者在查询用户时可以同时查询关联的用户详情信息。需要注意的是，`Preload`方法的参数是关联字段的名称，需要与模型中定义的字段名称一致。

- 通过预加载关联的记录，可以避免N+1查询问题，提高查询效率。预加载会在一次查询中获取主记录和关联记录，减少数据库的访问次数，提高性能。

- 注意，在使用`Preload`方法时，如果关联的记录不存在，Gorm会将关联字段设置为`nil`，而不是返回错误。因此在访问关联字段时需要进行空值检查，以避免出现空指针异常。

> *预加载*Preload() 的作用是：在查询主表数据时，提前把关联表数据也查出来，并自动填充到结构体的关联字段里。它解决的是 ORM 里的 关联数据预加载 / Eager Loading 问题。`Preload` 会通过 额外的 SQL 查询 加载关联数据；而 `Join Preload` 则是通过 LEFT JOIN 加载关联数据。

**删除**

```go
func delete() {
	// 删除用户详情会级联删除关联的用户
	err := db.Delete(&models.UserDetailModel{}, "user_id = ?", 1).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 删除用户会级联删除关联的用户详情
	err := db.Delete(&models.UserModel{}, "id = ?", 1).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 删除用户详情但不删除关联的用户
	err := db.Select("UserModel").Delete(&models.UserDetailModel{}, "user_id = ?", 1).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 也可以这样
	var detail models.UserDetailModel
	detail.UserID = 1
	db.Take(&detail) // 先查询出要删除的用户详情记录
	db.Delete(&detail) // 删除用户详情记录
	err := db.Model(&detail).Association("UserModel").Clear() // 清除关联关系，但不删除关联的用户记录
	if err != nil {
		log.Fatalln(err)
	}

	// 删除用户但不删除关联的用户详情
	err := db.Select("UserDetailModel").Delete(&models.UserModel{}, "id = ?", 1).Error
	if err != nil {
		log.Fatalln(err)
	}
}
```

- 在删除数据时，可以通过级联删除来同时删除关联的记录，或者通过选择性删除来只删除主记录或关联记录。需要注意的是，级联删除会自动处理关联记录的删除，而选择性删除需要使用`Select`方法来指定要保留的关联字段，保留的关联字段会被设置为`nil`，而不是删除关联记录。

也可以通过定义模型的`OnDelete`，`OnUpdate`标签来指定删除和更新的级联行为，例如：

```go
type UserDetailModel struct {
	ID       int
	UserID   int	     `gorm:"unique"` // 将UserID字段设置为唯一索引，这样才形成唯一的外键关系
	UserModel *UserModel `gorm:"foreignKey:UserID;constraint:OnDelete:CASCADE;"` // 定义一对一关系，指定外键为UserID，并设置删除时级联删除
	Email    string      `gorm:"size:32"`
}
```

- 通过在模型的关联字段上使用`constraint:OnDelete:CASCADE;`标签，可以指定当主记录被删除时，关联记录也会被自动删除，实现级联删除的效果。需要注意的是，这种级联删除是由数据库层面实现的，因此在使用时需要确保数据库支持级联删除，并且正确配置了外键约束。

- 如果不希望删除主记录时删除关联记录，可以使用`constraint:OnDelete:SET NULL;`标签来指定当主记录被删除时，将关联字段设置为`NULL`，而不是删除关联记录。例如：

