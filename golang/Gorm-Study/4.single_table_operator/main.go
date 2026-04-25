package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
	"log"

	"gorm.io/gorm"
)

func insert(user *models.UserModel) error {
	return global.DB.Create(user).Error
}

func queryAll() ([]models.UserModel, error) {
	var users []models.UserModel
	err := global.DB.Find(&users).Error
	if err != nil {
		return nil, err
	}
	fmt.Println(users)
	return users, nil
}

func query(id uint) (*models.UserModel, error) {
	var user models.UserModel
	if err := global.DB.First(&user, id).Error; err != nil {
		return nil, err
	}
	return &user, nil
}

func main() {
	global.Connect()
	// 1. 插入数据
	// user := &models.UserModel{
	// 	Name: "Alice",
	// 	Age:  30,
	// }
	// if err := insert(user); err != nil {
	// 	log.Fatalln(err)
	// }

	// 2. 查询数据
	if _, err := queryAll(); err != nil {
		fmt.Println(err)
	}

	// 3. 根据主键查询数据
	user, err := query(1)
	if err != nil {
		log.Fatalln(err)
	}
	fmt.Println(user)

	// 4. 错误查询
	user = &models.UserModel{}
	err = global.DB.Take(&user, 100).Error
	if err == gorm.ErrRecordNotFound {
		fmt.Println("Record not found")
	}
}
