package main

import (
	"Gorm-Study/global"
	"fmt"
	"log"
)

func main() {
	global.Connect()
	// 1. 原生 SQL 查询
	rows, err := global.DB.Raw("SELECT id, name FROM users WHERE age > ?", 30).Rows()
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

	// 2. 原生 SQL 执行
	result := global.DB.Exec("UPDATE users SET name = ? WHERE id = ?", "NewName", 1)
	if result.Error != nil {
		log.Fatalln("更新失败", result.Error)
	}
	rowsAffected := result.RowsAffected
	fmt.Printf("更新成功，受影响的行数: %d\n", rowsAffected)
}
