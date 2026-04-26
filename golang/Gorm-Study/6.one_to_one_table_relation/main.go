package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
	"log"
)

func createUser() {
	// 创建用户
	users := []models.UserModel{
		{Name: "Alice", Age: 30},
		{Name: "Bob", Age: 25},
		{Name: "Charlie", Age: 35},
		{Name: "David", Age: 28},
		{Name: "Eve", Age: 22},
	}
	err := global.DB.Create(users).Error
	if err != nil {
		panic("创建用户失败: " + err.Error())
	}

	// 创建用户详情（一对一）
	details := []models.UserDetailModel{
		{UserID: users[0].ID, Email: "alice@example.com"},
		{UserID: users[1].ID, Email: "bob@example.com"},
		{UserID: users[2].ID, Email: "charlie@example.com"},
		{UserID: users[3].ID, Email: "divid@example.com"},
		{UserID: users[4].ID, Email: "eve@example.com"},
	}

	if err := global.DB.Create(&details).Error; err != nil {
		panic("创建用户详情失败: " + err.Error())
	}
}

// 一对一关系插入数据
func insert() {
	// 创建用户并创建用户详情
	// err := global.DB.Create(&models.UserModel{
	// 	Name: "Helen",
	// 	Age:  29,
	// 	UserDetailModel: &models.UserDetailModel{
	// 		Email: "helen@example.com",
	// 	},
	// }).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }

	// 创建用户详情并创建用户
	err := global.DB.Create(&models.UserDetailModel{
		Email: "ann@example.com",
		UserModel: &models.UserModel{
			Name: "Ann",
			Age:  27,
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 创建用户并关联已有的用户详情
	// err := global.DB.Create(&models.UserModel{
	// 	Name: "Frank",
	// 	Age:  32,
	// 	UserDetailModel: &models.UserDetailModel{
	// 		ID: 1, // 假设已有的用户详情ID为1
	// 	},
	// }).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }

	// 创建用户详情并关联已有的用户
	// err := global.DB.Create(&models.UserDetailModel{
	// 	Email: "ana@example.com",
	// 	UserModel: &models.UserModel{
	// 		ID: 1, // 假设已有的用户ID为1
	// 	},
	// }).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }
}

func query() {
	var id = 5
	var detail models.UserDetailModel
	// 1. 通过用户详情查询关联的用户信息
	// 预加载用户详情关联的用户信息
	// Preload的第一个参数是关联字段的名称，这里是UserModel
	// Preload会在查询UserDetailModel时同时查询关联的UserModel，并将结果填充到UserModel字段中
	global.DB.Preload("UserModel").Take(&detail, "user_id = ?", id)
	fmt.Printf("用户详情: %+v\n", detail)
	fmt.Printf("关联的用户: %+v\n", detail.UserModel)

	// 2. 通过用户查询关联的用户详情信息
	var user models.UserModel
	global.DB.Preload("UserDetailModel").Take(&user, "id = ?", id)
	fmt.Printf("用户: %+v\n", user)
	fmt.Printf("关联的用户详情: %+v\n", user.UserDetailModel)
}

func delete() {
	// 删除用户详情会级联删除关联的用户
	// err := global.DB.Delete(&models.UserDetailModel{}, "user_id = ?", 1).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }

	// 删除用户会级联删除关联的用户详情
	err := global.DB.Delete(&models.UserModel{}, "id = ?", 1).Error
	if err != nil {
		log.Fatalln(err)
	}

	// 删除用户详情但不删除关联的用户
	// err := global.DB.Select("UserModel").Delete(&models.UserDetailModel{}, "user_id = ?", 1).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }

	// 删除用户但不删除关联的用户详情
	// err := global.DB.Select("UserDetailModel").Delete(&models.UserModel{}, "id = ?", 1).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }
}

func main() {
	global.Connect() // 连接数据库
	global.Migrate() // 自动迁移数据库
	// createUser()
	// insert()
	// query()
}
