package models

import (
	"database/sql/driver"
	"encoding/json"
	"errors"
	"fmt"
)

// 自定义数据类型需要实现 Scanner 和 Valuer 接口

type Card struct {
	Cars []string `json:"cars"`
}

type UserInfo struct {
	Like []string `json"like"`
}

type UserCustom struct {
	ID   uint
	Name string   `gorm:"size:32"`
	Info UserInfo `gorm:"type:longtext" json:"info"`
	Card Card     `gorm:"type:longtext;serializer:json" json:"card"` // 使用 GORM 内置的 JSON 序列化器
}

func (u *UserInfo) Scan(value interface{}) error {
	bytes, ok := value.([]byte)
	if !ok {
		errors.New(fmt.Sprint("Failed to unmarshal UserInfo value"))
	}
	res := UserInfo{}
	err := json.Unmarshal(bytes, &res)
	if err != nil {
		return err
	}
	*u = res
	return nil
}

func (u UserInfo) Value() (driver.Value, error) {
	return json.Marshal(u)
}
