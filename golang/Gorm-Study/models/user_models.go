package models

import (
	"errors"
	"fmt"
	"time"

	"gorm.io/gorm"
)

type UserModel struct {
	ID              int
	Name            string           `gorm:"not null;unique"`   // 将Name字段设置为主键
	Age             int              `gorm:"default:18"`        // 设置Age字段的默认值为18
	UserDetailModel *UserDetailModel `gorm:"foreignKey:UserID"` // 定义与UserDetailModel的关联关系，指定外键为UserID
	CreatedAt       time.Time        // 创建时间，gorm会自动设置这个字段的值为当前时间
	DeletedAt       gorm.DeletedAt   // 软删除字段，gorm会自动处理这个字段的值
}

type UserDetailModel struct {
	ID        int
	UserID    int
	UserModel *UserModel `gorm:"foreignKey:UserID"`
	Email     string     `gorm:"size:32"`
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
