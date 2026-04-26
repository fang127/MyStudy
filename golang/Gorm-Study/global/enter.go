package global

import (
	"Gorm-Study/models"
	"fmt"
	"log"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

var DB *gorm.DB

func Migrate() {
	err := DB.AutoMigrate(&models.UserModel{})
	err = DB.AutoMigrate(&models.UserDetailModel{})
	if err != nil {
		log.Fatalln(err)
	}
	fmt.Println("数据库迁移成功")
}

func Connect() {
	dsn := "harry:123456@tcp(127.0.0.1:3306)/gorm_study?charset=utf8mb4&parseTime=True"
	db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{
		DisableForeignKeyConstraintWhenMigrating: true,
	})
	if err != nil {
		log.Fatalln(err)
	}
	DB = db
}
