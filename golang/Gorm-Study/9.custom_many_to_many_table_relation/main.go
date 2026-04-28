package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"log"
	"time"
)

func UserManyToMany() {
	// 创建用户并且创建文章

	// 设置多对多关系的关联表，如果不设置，那么person_books表不会被关联上，不会填充自定义的字段
	global.DB.SetupJoinTable(&models.Person{}, "Books", &models.PersonBook{})

	err := global.DB.Create(&models.Person{
		Name: "李四",
		Age:  20,
		Books: []models.Book{
			{Title: "Python教程"},
			{Title: "Python进阶"},
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
}

func queryManyToMany() {
	// 该方法只能根据person查book，如法根据many2many的中间表查
	//var person models.Person
	// err := global.DB.Preload("Books").First(&person).Error
	// if err != nil {
	// 	log.Fatalln(err)
	// }
	// log.Printf("查询到的用户: %v\n", person)

	// 根据中间表查询，查询到person和book的关系
	type PersonCollBookResponse struct {
		Name        string
		Age         int
		BookID      int
		BookTitle   string
		DateCreated time.Time
	}

	var personID = 4
	var personBooks []models.PersonBook
	// 预加载Person和Book，查询person_id为2的记录，并且预加载关联的Person和Book数据
	global.DB.Preload("Persons").Preload("Books").Find(&personBooks, "person_id = ?", personID)
	for _, model := range personBooks {
		response := PersonCollBookResponse{
			Name:        model.Persons.Name,
			Age:         model.Persons.Age,
			BookID:      model.Books.ID,
			BookTitle:   model.Books.Title,
			DateCreated: model.CreatedAt,
		}
		log.Printf("查询到的用户和书籍关系: %v\n", response)
	}
}

func deleteManyToMany() {
	// 删除person_id为2的记录
	var personID = 2
	err := global.DB.Where("person_id = ?", personID).Delete(&models.PersonBook{}).Error
	if err != nil {
		log.Fatalln(err)
	}
}

func main() {
	global.Connect()
	// global.Migrate()
	// UserManyToMany()
	// queryManyToMany()
	deleteManyToMany()
}
