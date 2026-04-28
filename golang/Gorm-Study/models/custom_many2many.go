package models

import (
	"fmt"
	"time"

	"gorm.io/gorm"
)

type Person struct {
	ID    int
	Name  string `gorm:"size:32"`
	Age   int
	Books []Book `gorm:"many2many:person_books;JoinForeignKey:PersonID;JoinReferences:BookID"`
}

type Book struct {
	ID      int
	Title   string   `gorm:"size:32"`
	Persons []Person `gorm:"many2many:person_books;JoinForeignKey:BookID;JoinReferences:PersonID"`
}

// 自定义多对多关系表
type PersonBook struct {
	PersonID  int
	Persons   Person `gorm:"foreignKey:PersonID;references:ID"` // 这表示UserID是User表的外键，引用User表的ID字段
	BookID    int
	Books     Book      `gorm:"foreignKey:BookID;references:ID"` // 这表示BookID是Book表的外键，引用Book表的ID字段
	CreatedAt time.Time `json:"created_at"`
	Title     string    `gorm:"size:32" json:"title"`
}

func (PersonBook) TableName() string {
	return "person_books"
}

func (u *PersonBook) BeforeCreate(tx *gorm.DB) (err error) {
	fmt.Println("BeforeCreate hook called for PersonBook")
	var articleTitle string
	// 查询关联的Book的Title
	if err := tx.Model(&Book{}).Select("title").Where("id = ?", u.BookID).Scan(&articleTitle).Error; err != nil {
		return err
	}
	u.Title = articleTitle
	return nil
}
