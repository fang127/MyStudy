package models

import (
	"errors"
	"fmt"
	"time"

	"gorm.io/gorm"
)

type UserModel struct {
	ID        int
	Name      string    `gorm:"not null;unique"` // 将Name字段设置为主键
	Age       int       `gorm:"default:18"`      // 设置Age字段的默认值为18
	CreatedAt time.Time // 创建时间，gorm会自动设置这个字段的值为当前时间
}

// BeforeCreate是GORM提供的一个钩子函数，在创建记录之前会被调用
func (u *UserModel) BeforeCreate(tx *gorm.DB) (err error) {
	fmt.Println("hook func was called")
	// 可以在内部进行一些数据验证或自动填充字段的操作
	// u.Name = fmt.Sprintf("User_%d", time.Now().Unix()) // 生成一个唯一的用户名
	if u.Name == "" {
		return errors.New("Name cannot be empty")
	}
	return nil
}
