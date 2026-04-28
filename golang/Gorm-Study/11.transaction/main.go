package main

import (
	"Gorm-Study/global"
	"Gorm-Study/models"

	"gorm.io/gorm"
)

func main() {
	global.Connect()

	// 事务
	// 1. 使用 db.Transaction() 方法
	err := global.DB.Transaction(func(tx *gorm.DB) error {
		// 在这里执行数据库操作
		if err := tx.Create(&models.UserModel{Name: "Alice"}).Error; err != nil {
			return err // 返回错误会回滚事务
		}
		if err := tx.Create(&models.UserModel{Name: "Bob"}).Error; err != nil {
			return err // 返回错误会回滚事务
		}
		return nil // 返回 nil 会提交事务
	})
	if err != nil {
		// 处理错误
	}

	// 2. 手动控制事务
	tx := global.DB.Begin() // 开始事务
	if err := tx.Create(&models.UserModel{Name: "Charlie"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	if err := tx.Create(&models.UserModel{Name: "Dave"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	tx.Commit() // 提交事务

	// 3. 使用 SavePoint 和 RollbackTo
	tx = global.DB.Begin() // 开始事务
	if err := tx.Create(&models.UserModel{Name: "Eve"}).Error; err != nil {
		tx.Rollback() // 回滚事务
		return
	}
	tx.SavePoint("sp1") // 创建保存点
	if err := tx.Create(&models.UserModel{Name: "Frank"}).Error; err != nil {
		tx.RollbackTo("sp1") // 回滚到保存点
		return
	}
	tx.Commit() // 提交事务

	// 4. 使用嵌套事务
	// GORM 支持嵌套事务，内层事务失败时可以回滚到内层事务的保存点，而不会影响外层事务。
	// 注意：嵌套事务在某些数据库（如 MySQL）中可能不完全支持，具体行为取决于数据库的事务隔离级别和实现。
	err = global.DB.Transaction(func(tx *gorm.DB) error {
		if err := tx.Create(&models.UserModel{Name: "Grace"}).Error; err != nil {
			return err
		}
		return tx.Transaction(func(tx2 *gorm.DB) error {
			if err := tx2.Create(&models.UserModel{Name: "Heidi"}).Error; err != nil {
				return err
			}
			return nil
		})
	})
	if err != nil {
		// 处理错误
	}
}
