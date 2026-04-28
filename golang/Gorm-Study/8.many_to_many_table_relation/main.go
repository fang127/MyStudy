package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
	"log"
)

func ManyToMany() {
	// 创建一篇文章，新增tag
	global.DB.Create(&models.ArticleModel{
		Title: "Gorm-Study",
		Tags: []models.TagModel{
			{Title: "Gorm"},
			{Title: "Go"},
		},
	})
	// 创建一篇文章，选择已有的tag
	var tagIDLists = []int{2}
	var tagList []models.TagModel
	global.DB.Find(&tagList, "id in ?", tagIDLists)

	err := global.DB.Create(&models.ArticleModel{
		Title: "计算机学习",
		Tags:  tagList,
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
}

func query() {
	// 查文章时，把对应的标签查出来
	var articles []models.ArticleModel
	global.DB.Preload("Tags").Find(&articles)

	for _, article := range articles {
		log.Printf("文章：%s\n", article.Title)
		for _, tag := range article.Tags {
			log.Printf("标签：%s\n", tag.Title)
		}
	}

	fmt.Println("==============================================")

	// 查标签时，把对应的文章查出来
	var tags []models.TagModel
	global.DB.Preload("Articles").Find(&tags)

	for _, tag := range tags {
		log.Printf("标签：%s\n", tag.Title)
		for _, article := range tag.Articles {
			log.Printf("文章：%s\n", article.Title)
		}
	}
}

func update() {
	// 更新文章的标签
	// 将计算机学习的标签改为Gorm和Go
	var article models.ArticleModel
	global.DB.Preload("Tags").First(&article, "title = ?", "计算机学习")

	var tagIDLists = []int{1, 2}
	var tagList []models.TagModel
	global.DB.Find(&tagList, "id in ?", tagIDLists)
	global.DB.Model(&article).Association("Tags").Replace(tagList)

}

func delete() {
	// 删除文章时，关联关系会被删除，但标签不会被删除，因为这里没有级联删除，如果需要级联删除，需要在模型中设置gorm:"constraint:OnDelete:CASCADE;"
	var article models.ArticleModel
	global.DB.Preload("Tags").First(&article, "title = ?", "计算机学习")
	global.DB.Delete(&article)
}

func main() {
	global.Connect()
	global.Migrate()
	// ManyToMany()
	// query()
	// update()
	// delete()
}
