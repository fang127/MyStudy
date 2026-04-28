package models

type GirlModel struct {
	ID        int        `gorm:"primaryKey"`
	Name      string     `gorm:"size:16;not null;unique"`
	BoyModels []BoyModel `gorm:"foreignKey:GirlID"`
}

type BoyModel struct {
	ID        int    `gorm:"primaryKey"`
	Name      string `gorm:"size:16;not null;unique"`
	GirlID    int
	GirlModel GirlModel `gorm:"foreignKey:GirlID"`
}
