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

### 7. 联级关系

Gorm支持多种类型的关联关系，has one 表示一个模型拥有另一个模型；has many 表示一个模型拥有零个或多个另一个模型；belongs to 表示当前模型属于另一个模型；many to many 会在两个模型之间增加一张连接表。

| 关系             | 中文理解 | Go 字段形态                | 外键通常放在哪里 |
| -------------- | ---- | ---------------------- | -------- |
| `has one`      | 我有一个 | `Profile Profile`      | 对方表里     |
| `has many`     | 我有多个 | `Orders []Order`       | 对方表里     |
| `belongs to`   | 我属于谁 | `User User` + `UserID` | 当前表里     |
| `many to many` | 多对多  | `Roles []Role`         | 中间表里     |

下面用一套电商/用户系统模型，把 GORM 中四种关系讲清楚：

| 关系             | 中文理解 | Go 字段形态                | 外键通常放在哪里 |
| -------------- | ---- | ---------------------- | -------- |
| `has one`      | 我有一个 | `Profile Profile`      | 对方表里     |
| `has many`     | 我有多个 | `Orders []Order`       | 对方表里     |
| `belongs to`   | 我属于谁 | `User User` + `UserID` | 当前表里     |
| `many to many` | 多对多  | `Roles []Role`         | 中间表里     |

GORM 官方文档中，`has one` 表示一个模型拥有另一个模型；`has many` 表示一个模型拥有零个或多个另一个模型；`belongs to` 表示当前模型属于另一个模型；`many to many` 会在两个模型之间增加一张连接表。([GORM][1])

---

#### 7.1 `has one`：一对一，我有一个

1. 业务例子

一个用户有一个用户资料：

```text
users
  id
  name

profiles
  id
  user_id
  avatar
  bio
```

关系是：

```text
User has one Profile
```

也就是：

```text
一个 User 拥有一个 Profile
```

2. 模型定义

```go
type User struct {
    ID      uint
    Name    string
    Profile Profile
}

type Profile struct {
    ID     uint
    UserID uint
    Avatar string
    Bio    string
}
```

重点是：

```go
Profile Profile
```

表示 `User` 有一个 `Profile`。

而外键在 `profiles` 表里：

```go
UserID uint
```

GORM 默认会用：

```text
User.ID -> Profile.UserID
```

官方 `has one` 示例也是类似：`User` 拥有一个 `CreditCard`，而 `CreditCard` 里有 `UserID` 外键。([GORM][1])

3. 自动建表

```go
err := db.AutoMigrate(&User{}, &Profile{})
```

大致对应表结构：

```sql
CREATE TABLE users (
    id bigint unsigned AUTO_INCREMENT PRIMARY KEY,
    name varchar(255)
);

CREATE TABLE profiles (
    id bigint unsigned AUTO_INCREMENT PRIMARY KEY,
    user_id bigint unsigned,
    avatar varchar(255),
    bio text
);
```

4. 创建数据

- 方式一：创建用户时顺带创建 Profile

```go
user := User{
    Name: "Tom",
    Profile: Profile{
        Avatar: "tom.png",
        Bio:    "Golang developer",
    },
}

err := db.Create(&user).Error
```

GORM 会先创建 `users`，拿到 `user.ID` 后，再创建 `profiles`，并把 `profiles.user_id` 设置成 `users.id`。

- 方式二：先创建 User，再创建 Profile

```go
user := User{Name: "Tom"}
err := db.Create(&user).Error
if err != nil {
    return err
}

profile := Profile{
    UserID: user.ID,
    Avatar: "tom.png",
    Bio:    "Golang developer",
}

err = db.Create(&profile).Error
```

这种写法最直观，适合业务代码里明确控制创建顺序。

5. 查询并预加载

如果直接查：

```go
var user User
db.First(&user, 1)
```

只会查 `users` 表，`Profile` 不会自动填充。

要加载关联：

```go
var user User

err := db.Preload("Profile").
    First(&user, 1).Error
```

大致 SQL：

```sql
SELECT * FROM users WHERE id = 1 ORDER BY id LIMIT 1;

SELECT * FROM profiles WHERE user_id = 1;
```

`Preload("Profile")` 里的 `"Profile"` 是结构体字段名，不是表名。

6. 自定义外键

默认情况下，GORM 会找：

```go
Profile.UserID
```

如果你的字段不是 `UserID`，比如叫 `OwnerID`：

```go
type User struct {
    ID      uint
    Name    string
    Profile Profile `gorm:"foreignKey:OwnerID"`
}

type Profile struct {
    ID      uint
    OwnerID uint
    Avatar  string
}
```

含义是：

```text
User.ID -> Profile.OwnerID
```

官方文档说明，`has one` 可以通过 `foreignKey` 指定关联字段；默认外键名通常是拥有者类型名加主键名，例如 `UserID`。

7. 自定义关联参考字段

如果不用 `User.ID`，而用 `User.MemberNo` 关联：

```go
type User struct {
    ID       uint
    MemberNo string `gorm:"uniqueIndex"`
    Profile  Profile `gorm:"foreignKey:UserMemberNo;references:MemberNo"`
}

type Profile struct {
    ID           uint
    UserMemberNo string
    Avatar       string
}
```

含义：

```text
User.MemberNo -> Profile.UserMemberNo
```

#### 7.2 `has many`：一对多，我有多个

1. 业务例子

一个用户有多个订单：

```text
users
  id
  name

orders
  id
  user_id
  order_no
  amount
```

关系是：

```text
User has many Orders
```

也就是：

```text
一个用户拥有多个订单
```

2. 模型定义

```go
type User struct {
    ID     uint
    Name   string
    Orders []Order
}

type Order struct {
    ID      uint
    UserID  uint
    OrderNo string
    Amount  int64
}
```

重点是：

```go
Orders []Order
```

表示一个用户有多个订单。

外键仍然在对方表，也就是 `orders` 表里：

```go
UserID uint
```

GORM 默认规则：

```text
User.ID -> Order.UserID
```

官方 `has many` 文档说明，`has many` 是一对多关系，拥有者可以有零个或多个另一个模型实例；默认外键名是拥有者类型名加主键字段名，例如 `UserID`。([GORM][2])

---

3. 创建数据

- 方式一：创建 User 时一起创建 Orders

```go
user := User{
    Name: "Tom",
    Orders: []Order{
        {
            OrderNo: "A001",
            Amount:  1000,
        },
        {
            OrderNo: "A002",
            Amount:  2000,
        },
    },
}

err := db.Create(&user).Error
```

GORM 会自动把订单的 `UserID` 设置为 `user.ID`。

- 方式二：先创建 User，再创建 Orders

```go
user := User{Name: "Tom"}
err := db.Create(&user).Error
if err != nil {
    return err
}

orders := []Order{
    {
        UserID:  user.ID,
        OrderNo: "A001",
        Amount:  1000,
    },
    {
        UserID:  user.ID,
        OrderNo: "A002",
        Amount:  2000,
    },
}

err = db.Create(&orders).Error
```

这种方式在实际项目中更常见，尤其是订单创建通常有独立业务流程。

4. 查询并预加载

```go
var users []User

err := db.Preload("Orders").
    Find(&users).Error
```

大致 SQL：

```sql
SELECT * FROM users;

SELECT * FROM orders WHERE user_id IN (1,2,3,...);
```

GORM 官方 `has many` 示例也是使用 `Preload("CreditCards")` 加载一对多关联。([GORM][2])

5. 带条件预加载

比如只预加载已支付订单：

```go
var users []User

err := db.Preload("Orders", "status = ?", "paid").
    Find(&users).Error
```

假设模型是：

```go
type Order struct {
    ID      uint
    UserID  uint
    OrderNo string
    Amount  int64
    Status  string
}
```

大致 SQL：

```sql
SELECT * FROM users;

SELECT * FROM orders
WHERE user_id IN (1,2,3,...)
  AND status = 'paid';
```

注意：这只是限制预加载的 `Orders`，不是过滤 `users`。

也就是说：

```go
db.Preload("Orders", "status = ?", "paid").Find(&users)
```

含义是：

```text
查所有用户，但每个用户下面只填充 paid 订单。
```

不是：

```text
只查有 paid 订单的用户。
```

如果要过滤主表用户，应该配合 `Joins` 或 `EXISTS`：

```go
err := db.
    Joins("JOIN orders ON orders.user_id = users.id").
    Where("orders.status = ?", "paid").
    Preload("Orders", "status = ?", "paid").
    Find(&users).Error
```

GORM 预加载文档说明，`Preload` 可以带条件，并且会分别执行主表查询和关联表查询。([GORM][3])

6. 自定义外键

如果订单表里不叫 `UserID`，而叫 `OwnerID`：

```go
type User struct {
    ID     uint
    Name   string
    Orders []Order `gorm:"foreignKey:OwnerID"`
}

type Order struct {
    ID      uint
    OwnerID uint
    OrderNo string
}
```

含义：

```text
User.ID -> Order.OwnerID
```

7. 自定义参考字段

如果用 `User.MemberNo` 关联订单：

```go
type User struct {
    ID       uint
    MemberNo string `gorm:"uniqueIndex"`
    Orders   []Order `gorm:"foreignKey:UserMemberNo;references:MemberNo"`
}

type Order struct {
    ID           uint
    UserMemberNo string
    OrderNo      string
}
```

含义：

```text
User.MemberNo -> Order.UserMemberNo
```

官方 `has many` 文档也说明，可以用 `foreignKey` 自定义外键，用 `references` 指定使用拥有者的哪个字段作为关联值。

#### 7.3 `belongs to`：我属于谁

1. 业务例子

订单属于一个用户：

```text
orders
  id
  user_id
  order_no
  amount

users
  id
  name
```

关系是：

```text
Order belongs to User
```

也就是：

```text
订单属于用户
```

2. 模型定义

```go
type Order struct {
    ID      uint
    UserID  uint
    User    User
    OrderNo string
    Amount  int64
}

type User struct {
    ID   uint
    Name string
}
```

重点是当前模型 `Order` 里有：

```go
UserID uint
User   User
```

这里外键在当前表，也就是 `orders.user_id`。

GORM 默认规则：

```text
Order.UserID -> User.ID
```

官方 `belongs to` 文档中也是类似：`User` 属于 `Company`，所以 `User` 结构体中包含 `CompanyID` 和 `Company` 字段，默认用 `CompanyID` 创建外键关系。([GORM][4])

---

3. `belongs to` 和 `has many` 是同一关系的两个方向

这两个模型可以同时写：

```go
type User struct {
    ID     uint
    Name   string
    Orders []Order
}

type Order struct {
    ID      uint
    UserID  uint
    User    User
    OrderNo string
    Amount  int64
}
```

从 `User` 看：

```text
User has many Orders
```

从 `Order` 看：

```text
Order belongs to User
```

数据库里其实还是一个外键：

```text
orders.user_id -> users.id
```

4. 创建数据

- 方式一：已有用户，创建订单

```go
order := Order{
    UserID:  1,
    OrderNo: "A001",
    Amount:  1000,
}

err := db.Create(&order).Error
```

- 方式二：创建订单时带 User

```go
order := Order{
    User: User{
        Name: "Tom",
    },
    OrderNo: "A001",
    Amount:  1000,
}

err := db.Create(&order).Error
```

GORM 会先创建 `User`，然后把生成的 `User.ID` 写入 `Order.UserID`。

实际业务里，如果用户已经存在，更推荐直接传 `UserID`，避免误创建用户。

5. 查询并预加载

查询订单，同时加载用户：

```go
var orders []Order

err := db.Preload("User").
    Find(&orders).Error
```

大致 SQL：

```sql
SELECT * FROM orders;

SELECT * FROM users WHERE id IN (1,2,3,...);
```

6. 单个订单加载用户

```go
var order Order

err := db.Preload("User").
    First(&order, "order_no = ?", "A001").Error
```

使用结果：

```go
fmt.Println(order.OrderNo)
fmt.Println(order.User.Name)
```

7. 自定义外键

如果订单里不是 `UserID`，而是 `BuyerID`：

```go
type Order struct {
    ID      uint
    BuyerID uint
    Buyer   User `gorm:"foreignKey:BuyerID"`
    OrderNo string
}

type User struct {
    ID   uint
    Name string
}
```

含义：

```text
Order.BuyerID -> User.ID
```

8. 自定义参考字段

如果订单里存的是用户编号，不是用户 ID：

```go
type Order struct {
    ID           uint
    BuyerCode    string
    Buyer        User `gorm:"foreignKey:BuyerCode;references:MemberNo"`
    OrderNo      string
}

type User struct {
    ID       uint
    MemberNo string `gorm:"uniqueIndex"`
    Name     string
}
```

含义：

```text
Order.BuyerCode -> User.MemberNo
```

官方文档说明，`belongs to` 默认使用被归属模型的主键作为外键值，也可以用 `references` 改成其他字段；如果自定义外键名在拥有者类型里已经存在，GORM 可能会猜成 `has one`，这时需要显式指定 `references`。([GORM][4])

#### 7.4 `many to many`：多对多

1. 业务例子

一个用户可以有多个角色，一个角色也可以属于多个用户：

```text
users
  id
  name

roles
  id
  name

user_roles
  user_id
  role_id
```

关系是：

```text
User many to many Role
```

也就是：

```text
用户和角色多对多
```

2. 模型定义

```go
type User struct {
    ID    uint
    Name  string
    Roles []Role `gorm:"many2many:user_roles;"`
}

type Role struct {
    ID   uint
    Name string
}
```

重点是：

```go
Roles []Role `gorm:"many2many:user_roles;"`
```

这表示 GORM 会使用中间表：

```text
user_roles
```

默认中间表字段大致是：

```text
user_id
role_id
```

官方 `many to many` 文档说明，多对多会在两个模型之间增加 join table；例如 `User` 和 `Language` 之间通过 `user_languages` 连接，`AutoMigrate` 时 GORM 会自动创建连接表。([GORM][5])

3. 自动建表

```go
err := db.AutoMigrate(&User{}, &Role{})
```

GORM 会创建：

```text
users
roles
user_roles
```

中间表大致：

```sql
CREATE TABLE user_roles (
    user_id bigint unsigned,
    role_id bigint unsigned
);
```

实际生成细节和数据库、约束配置有关。

4. 创建用户并绑定角色

```go
user := User{
    Name: "Tom",
    Roles: []Role{
        {Name: "admin"},
        {Name: "editor"},
    },
}

err := db.Create(&user).Error
```

GORM 会创建：

```text
users 记录
roles 记录
user_roles 关联记录
```

5. 已有角色，给用户绑定角色

```go
var user User
err := db.First(&user, 1).Error
if err != nil {
    return err
}

var roles []Role
err = db.Where("name IN ?", []string{"admin", "editor"}).
    Find(&roles).Error
if err != nil {
    return err
}

err = db.Model(&user).
    Association("Roles").
    Append(&roles)
```

这会往 `user_roles` 中插入关联记录。

6. 查询用户并预加载角色

```go
var users []User

err := db.Preload("Roles").
    Find(&users).Error
```

大致 SQL：

```sql
SELECT * FROM users;

SELECT * FROM user_roles WHERE user_id IN (1,2,3,...);

SELECT * FROM roles WHERE id IN (1,2,3,...);
```

官方文档中的多对多查询也是通过 `Preload("Languages")` 预加载关联。([GORM][5])

7. 反向关联

如果你也想从角色查用户：

```go
type User struct {
    ID    uint
    Name  string
    Roles []*Role `gorm:"many2many:user_roles;"`
}

type Role struct {
    ID    uint
    Name  string
    Users []*User `gorm:"many2many:user_roles;"`
}
```

查询角色时加载用户：

```go
var roles []Role

err := db.Preload("Users").
    Find(&roles).Error
```

官方文档也给了 back-reference 示例：两个模型都声明同一个 `many2many` join table 后，可以从任一侧预加载另一侧。

8. 自定义中间表字段

默认：

```text
user_roles.user_id -> users.id
user_roles.role_id -> roles.id
```

如果你想用自定义字段，比如：

```go
type User struct {
    ID      uint
    Refer   uint `gorm:"uniqueIndex"`
    Roles   []Role `gorm:"many2many:user_roles;foreignKey:Refer;joinForeignKey:UserReferID;references:Code;joinReferences:RoleCode"`
}

type Role struct {
    ID   uint
    Code string `gorm:"uniqueIndex"`
    Name string
}
```

含义：

```text
user_roles.user_refer_id -> users.refer
user_roles.role_code     -> roles.code
```

GORM 官方文档说明，`many2many` 的外键在 join table 中，可以用 `foreignKey`、`references`、`joinForeignKey`、`joinReferences` 自定义。([GORM][5])

#### 7.5 Association Mode：操作关联关系

GORM 还有 `Association` 模式，适合专门处理关联数据。官方文档说明，Association Mode 提供了处理模型之间关系的辅助方法，使用时需要指定源模型和关联字段名。([GORM][6])

1. 给用户追加角色

```go
var user User
db.First(&user, 1)

var role Role
db.First(&role, "name = ?", "admin")

err := db.Model(&user).
    Association("Roles").
    Append(&role)
```

2. 替换用户角色

```go
var user User
db.First(&user, 1)

var roles []Role
db.Where("name IN ?", []string{"admin", "editor"}).Find(&roles)

err := db.Model(&user).
    Association("Roles").
    Replace(&roles)
```

`Replace` 会把原来的关联替换为新的关联。

3. 删除某个关联

```go
var user User
db.First(&user, 1)

var role Role
db.First(&role, "name = ?", "editor")

err := db.Model(&user).
    Association("Roles").
    Delete(&role)
```

注意：对于多对多，这通常是删除中间表关联，不是删除 `roles` 表里的角色记录。

4. 清空关联

```go
var user User
db.First(&user, 1)

err := db.Model(&user).
    Association("Roles").
    Clear()
```

5. 统计关联数量

```go
var user User
db.First(&user, 1)

count := db.Model(&user).
    Association("Roles").
    Count()
```

#### 7.6 `Preload` 和 `Joins` 怎么选？

1. `Preload`

```go
db.Preload("Orders").Find(&users)
```

特点：

```text
多条 SQL
适合 has many、many to many
不会因为一对多导致主表重复行
```

2. `Joins`

```go
db.Joins("Profile").Find(&users)
```

特点：

```text
JOIN SQL
适合 has one、belongs to
```

### 8. 自定义数据类型 - 序列化

GORM 允许你定义自定义数据类型，并实现 `Scanner` 和 `Valuer` 接口来控制如何将数据从数据库扫描到 Go 结构体，以及如何将 Go 结构体的值保存到数据库中。

例如，定义一个 `JSONMap` 类型来存储 JSON 数据：

```go
type JSONMap map[string]interface{}

func (m JSONMap) Value() (driver.Value, error) {
    if len(m) == 0 {
        return "{}", nil
    }
    return json.Marshal(m)
}

func (m *JSONMap) Scan(value interface{}) error {
    if value == nil {
        *m = make(JSONMap)
        return nil
    }
    bytes, ok := value.([]byte)
    if !ok {
        return fmt.Errorf("failed to scan JSONMap: %v", value)
    }
    return json.Unmarshal(bytes, m)
}
```

在模型中使用自定义类型：

```go
type User struct {
    ID       uint
    Name     string
    Metadata JSONMap `gorm:"type:json"`
}
```

- 在这个例子中，`JSONMap` 实现了 `Value` 方法来将 Go 的 `map[string]interface{}` 转换为 JSON 字符串存储到数据库中，同时实现了 `Scan` 方法来将数据库中的 JSON 字符串转换回 Go 的 `map[string]interface{}`。

GORM 还支持自定义序列化器，可以在模型字段上使用 `serializer` 标签来指定一个实现了 `Serializer` 接口的类型，用于控制该字段的序列化和反序列化过程。例如：

```go
type User struct {
    ID       uint
    Name     string
    Metadata JSONMap `gorm:"serializer:json"`
}
```

- 在这个例子中，`Metadata` 字段使用了 `json` 序列化器，GORM 会自动将该字段的值序列化为 JSON 格式存储到数据库中，并在查询时反序列化回 Go 的类型。

也可以自定义序列化器：

一个Serializer需要实现如何对数据进行序列化和反序列化，所以需要实现如下接口

```go
import "gorm.io/gorm/schema"

type SerializerInterface interface {
    Scan(ctx context.Context, field *schema.Field, dst reflect.Value, dbValue interface{}) error
    SerializerValuerInterface
}

type SerializerValuerInterface interface {
    Value(ctx context.Context, field *schema.Field, dst reflect.Value, fieldValue interface{}) (interface{}, error)
}
例如，默认 JSONSerializer 的实现如下：

// JSONSerializer json serializer
type JSONSerializer struct {
}

// Scan implements serializer interface
func (JSONSerializer) Scan(ctx context.Context, field *schema.Field, dst reflect.Value, dbValue interface{}) (err error) {
    fieldValue := reflect.New(field.FieldType)

    if dbValue != nil {
        var bytes []byte
        switch v := dbValue.(type) {
        case []byte:
            bytes = v
        case string:
            bytes = []byte(v)
        default:
            return fmt.Errorf("failed to unmarshal JSONB value: %#v", dbValue)
        }

        err = json.Unmarshal(bytes, fieldValue.Interface())
    }

    field.ReflectValueOf(ctx, dst).Set(fieldValue.Elem())
    return
}

// Value implements serializer interface
func (JSONSerializer) Value(ctx context.Context, field *Field, dst reflect.Value, fieldValue interface{}) (interface{}, error) {
    return json.Marshal(fieldValue)
}
```

并使用以下代码注册：

```go
schema.RegisterSerializer("json", JSONSerializer{})
```

注册序列化器后，您可以将其与 serializer 标签一起使用，例如：

```go
type User struct {
    Name []byte `gorm:"serializer:json"`
}
```

### 9. 自定义数据类型 - 枚举

GORM 也支持自定义枚举类型，可以通过实现 `Scanner` 和 `Valuer` 接口来定义枚举类型的行为。例如，定义一个 `LevelInfo` 枚举类型：

```go
package models

import "encoding/json"

type Level int8

const (
	InfoLevel  Level = 1
	WarnLevel  Level = 2
	ErrorLevel Level = 3
)

type LogModel struct {
	ID    uint   `json:"id" gorm:"primaryKey"`
	Title string `gorm:"size:32"`
	Level Level  `json"level"`
}

func (l LogModel) MarshalJSON() ([]byte, error) {
	var str string
	switch l.Level {
	case InfoLevel:
		str = "info"
	case WarnLevel:
		str = "warn"
	case ErrorLevel:
		str = "error"
	default:
		str = "unknown"
	}
	return json.Marshal(str)
}
```

- 在这个例子中，`Level` 是一个自定义的枚举类型，定义了三个级别：`InfoLevel`、`WarnLevel` 和 `ErrorLevel`。在 `LogModel` 结构体中，`Level` 字段使用了自定义的枚举类型，并实现了 `MarshalJSON` 方法来控制 JSON 序列化时的输出格式。

- 注意，MarshalJSON 方法只是控制 JSON 序列化的输出格式，如果需要控制数据库存储的值，还需要实现 `Scanner` 和 `Valuer` 接口来定义枚举类型在数据库中的存储和读取行为。

### 10. 事务

GORM 提供了事务支持，可以通过 `db.Transaction` 方法来执行一系列数据库操作，并确保它们要么全部成功，要么全部回滚。例如：

```go
package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"

	"gorm.io/gorm"
)

func main() {
	global.Connect()

	// 事务
	// 1. 使用 db.Transaction() 方法
	err := global.DB.Transaction(func(tx *gorm.DB) error {
		// 在这里执行数据库操作
		if err := tx.Create(&models.UserModel{Name: "Alice"}).Error; err != nil {
			return err // 返回错误会回滚事务
		}
		if err := tx.Create(&models.UserModel{Name: "Bob"}).Error; err != nil {
			return err // 返回错误会回滚事务
		}
		return nil // 返回 nil 会提交事务
	})
	if err != nil {
		// 处理错误
	}

	// 2. 手动控制事务
	tx := global.DB.Begin() // 开始事务
	if err := tx.Create(&models.UserModel{Name: "Charlie"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	if err := tx.Create(&models.UserModel{Name: "Dave"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	tx.Commit() // 提交事务

	// 3. 使用 SavePoint 和 RollbackTo
	tx = global.DB.Begin() // 开始事务
	if err := tx.Create(&models.UserModel{Name: "Eve"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	tx.SavePoint("sp1") // 创建保存点
	if err := tx.Create(&models.UserModel{Name: "Frank"}).Error; err != nil {
		tx.RollbackTo("sp1") // 回滚到保存点
		return
	}
	tx.Commit() // 提交事务

	// 4. 使用嵌套事务
	// GORM 支持嵌套事务，内层事务失败时可以回滚到内层事务的保存点，而不会影响外层事务。
	// 注意：嵌套事务在某些数据库（如 MySQL）中可能不完全支持，具体行为取决于数据库的事务隔离级别和实现。
	err = global.DB.Transaction(func(tx *gorm.DB) error {
		if err := tx.Create(&models.UserModel{Name: "Grace"}).Error; err != nil {
			return err
		}
		return tx.Transaction(func(tx2 *gorm.DB) error {
			if err := tx2.Create(&models.UserModel{Name: "Heidi"}).Error; err != nil {
				return err
			}
			return nil
		})
	})
	if err != nil {
		// 处理错误
	}
}
```

- 在这个例子中，展示了四种使用事务的方式：使用 `db.Transaction` 方法、手动控制事务、使用保存点以及嵌套事务。每种方式都确保了在发生错误时能够正确回滚事务，以保持数据的一致性。

### 11. Gorm 配置

GORM 提供了多种配置选项，可以通过 `gorm.Config` 结构体来设置。例如：

```go
gormConfig := &gorm.Config{
    Logger: logger.Default.LogMode(logger.Info), // 设置日志级别
    NamingStrategy: schema.NamingStrategy{
        TablePrefix:   "prefix_", // 表名前缀
        SingularTable: true,      // 使用单数表名
    },
    DisableForeignKeyConstraintWhenMigrating: true, // 迁移时禁用外键约束
}
db, err := gorm.Open(mysql.Open(dsn), gormConfig)
if err != nil {
    log.Fatalln("连接数据库失败", err)
}
```

- 在这个例子中，创建了一个 `gorm.Config` 实例，并设置了日志级别、命名策略以及迁移时的外键约束行为。然后在打开数据库连接时将这个配置传入。

- GORM 的配置选项非常丰富，可以根据需要进行调整，以满足不同的应用场景和需求。

- 具体的配置选项可以参考 GORM 官方文档中的 [Config](https://gorm.io/zh_CN/docs/gorm_config.html) 部分，了解更多关于日志、命名策略、迁移行为等方面的配置细节。


## 总结

GORM 是一个功能强大且易于使用的 Go 语言 ORM 框架，提供了丰富的功能来简化数据库操作。通过本文的介绍，我们了解了 GORM 的基本使用方法、模型定义、查询构建器、原生 SQL 执行、迁移、关联关系、自定义数据类型以及事务处理等方面的内容。希望这些示例和说明能够帮助你更好地理解和使用 GORM 来构建高效的 Go 应用程序。