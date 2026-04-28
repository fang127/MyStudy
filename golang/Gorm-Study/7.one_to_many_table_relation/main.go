package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"
	"fmt"
	"log"
)

func ontToMany() {
	// 一个女神，自带两舔狗
	err := global.DB.Create(&models.GirlModel{
		Name: "小璐",
		BoyModels: []models.BoyModel{
			{Name: "小明"},
			{Name: "小强"},
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
	// 来了个女神，选了个舔狗
	err = global.DB.Create(&models.GirlModel{
		Name: "小张",
		BoyModels: []models.BoyModel{
			{ID: 1},
		},
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
	// 来了一个舔狗，选了个女神
	err = global.DB.Create(&models.BoyModel{
		Name: "小李",
		// GirlModel: models.GirlModel{
		// 	ID: 2,
		// },

		// 也可以这样写
		GirlID: 2,
	}).Error
	if err != nil {
		log.Fatalln(err)
	}
}

func query() {
	// 查询小张的舔狗
	var girl models.GirlModel
	global.DB.Preload("BoyModels").Take(&girl, "name = ?", "小张")
	fmt.Println(girl.Name, " ", girl.BoyModels, " ", len(girl.BoyModels))
	// 预加载设置条件
	global.DB.Preload("BoyModels", "name = ?", "小李").Take(&girl, "name = ?", "小张")
	fmt.Println(girl.Name, " ", girl.BoyModels, " ", len(girl.BoyModels))

	// 专门查关联
	girl = models.GirlModel{}
	// 先查女神，再查舔狗
	global.DB.Take(&girl, "name = ?", "小张")
	// 通过Association方法查询关联
	var boyModels []models.BoyModel
	global.DB.Model(&girl).Association("BoyModels").Find(&boyModels)
	fmt.Println(girl.Name, " ", boyModels, " ", len(boyModels))
}

func modify() {
	// 修改小张的舔狗
	var girl models.GirlModel
	global.DB.Take(&girl, "name = ?", "小张")
	// 该操作会删除原有关联，重新创建关联
	global.DB.Model(&girl).Association("BoyModels").Replace([]models.BoyModel{
		{Name: "小王"},
		{Name: "小赵"},
	})

	// 小张的舔狗都不舔了
	global.DB.Model(&girl).Association("BoyModels").Clear()

	// 1，3号舔狗又舔了小张
	global.DB.Model(&girl).Association("BoyModels").Append([]models.BoyModel{
		{ID: 1},
		{ID: 3},
	})

	// 只有3号退出舔狗行列
	global.DB.Model(&girl).Association("BoyModels").Delete(&models.BoyModel{ID: 3})
}

func main() {
	global.Connect()
	global.Migrate()
	// ontToMany()
	// query()
	modify()
}
