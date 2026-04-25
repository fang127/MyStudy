package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
)

func migrate() {
	err := global.DB.AutoMigrate(&models.UserModel{})
	if err != nil {
		fmt.Println("AutoMigrate failed:", err)
		return
	}
	fmt.Println("AutoMigrate successful")
}

func main() {
	global.Connect()
	fmt.Println(global.DB)
	migrate()
}
