package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"encoding/json"
	"fmt"
)

func create() {
	err := global.DB.Create(&models.UserCustom{
		Name: "小李",
		Info: models.UserInfo{
			Like: []string{"篮球", "足球"},
		},
		Card: models.Card{
			Cars: []string{"奔驰", "宝马"},
		},
	}).Error
	if err != nil {
		fmt.Println(err)
	}
}

func query() {
	var user models.UserCustom
	global.DB.Where("name = ?", "小李").Take(&user)
	fmt.Println(user)
	fmt.Println(user.Info.Like)
	fmt.Println(user.Card.Cars)
}

func createLogLevel() {
	global.DB.Create(&models.LogModel{
		Title: "日志1",
		Level: models.InfoLevel,
	})
}

func queryLogLevel() {
	//
	var log models.LogModel
	global.DB.Take(&log)
	byteData, _ := json.Marshal(log)
	fmt.Println(string(byteData))
}

func main() {
	global.Connect()
	// global.Migrate()
	// create()
	// query()

	createLogLevel()
	queryLogLevel()
}
