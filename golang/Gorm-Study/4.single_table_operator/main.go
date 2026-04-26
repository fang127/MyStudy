package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
	"time"

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

func updateSave() {
	var user models.UserModel
	user.ID = 3
	user.Name = "Bob"
	user.Age = 25
	user.CreatedAt = time.Now()
	global.DB.Save(&user) // 更新所有字段，包括零值字段
	fmt.Println(user)
}

func update() {
	var user models.UserModel
	user.ID = 3
	user.Name = "Bob"
	user.Age = 25
	global.DB.Model(&user).Updates(user) // 更新非零值字段
	fmt.Println(user)
}

func updates() {
	var user models.UserModel
	user.ID = 3
	user.Name = "Bob"
	user.Age = 25
	global.DB.Model(&user).Updates(map[string]interface{}{
		"Name":      user.Name,
		"Age":       user.Age,
		"CreatedAt": time.Now(),
	}) // 更新指定字段
	fmt.Println(user)
}

func updateColumn() {
	var user models.UserModel
	user.ID = 3
	user.Name = "Bob"
	user.Age = 25
	global.DB.Model(&user).UpdateColumn("Name", user.Name) // 更新单个字段，允许零值
	fmt.Println(user)
}

func delete() {
	// var user models.UserModel
	// user.ID = 3
	// global.DB.Delete(&user) // 根据主键删除
	// fmt.Println(user)

	// global.DB.Where("age > ?", 30).Delete(&models.UserModel{}) // 根据条件删除

	global.DB.Delete(&models.UserModel{}, 3) // 直接根据主键删除
}

func deleteAll() {
	global.DB.Where("1 = 1").Delete(&models.UserModel{}) // 删除所有记录
}

// 分页查询
func pageQuery(page, pageSize int) ([]models.UserModel, error) {
	var users []models.UserModel
	err := global.DB.Offset((page - 1) * pageSize).Limit(pageSize).Find(&users).Error
	if err != nil {
		return nil, err
	}
	fmt.Println(users)
	return users, nil
}

// Scope查询
func scopeQuery(age int) ([]models.UserModel, error) {
	var users []models.UserModel
	// 定义一个Scope函数，接受一个gorm.DB对象并返回一个新的gorm.DB对象
	err := global.DB.Scopes(func(db *gorm.DB) *gorm.DB {
		return db.Where("age > ?", age)
	}).Find(&users).Error
	if err != nil {
		return nil, err
	}
	fmt.Println(users)
	return users, nil
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
	// if _, err := queryAll(); err != nil {
	// 	fmt.Println(err)
	// }

	// // 3. 根据主键查询数据
	// user, err := query(1)
	// if err != nil {
	// 	log.Fatalln(err)
	// }
	// fmt.Println(user)

	// // 4. 错误查询
	// user = &models.UserModel{}
	// err = global.DB.Take(&user, 100).Error
	// if err == gorm.ErrRecordNotFound {
	// 	fmt.Println("Record not found")
	// }

	// 5. 更新数据
	// updateSave()
}
