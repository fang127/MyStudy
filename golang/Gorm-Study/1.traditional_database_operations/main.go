package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/go-sql-driver/mysql" // 导入MySQL驱动，使用匿名导入方式，因为我们只需要注册驱动，不需要直接使用包中的函数。
)

// 传统方法操作数据库，使用sql包，手动编写SQL语句，执行查询和更新操作。
func main() {
	// 1. 连接数据库
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
}
