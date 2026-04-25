package main

import (
	"fmt"
	"log"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

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
