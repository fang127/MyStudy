package models

type ArticleModel struct {
	ID    int
	Title string     `gorm:"size:32;unique;not null"`
	Tags  []TagModel `gorm:"many2many:article_tags;"`
}

type TagModel struct {
	ID       int
	Title    string         `gorm:"size:32;unique;not null"`
	Articles []ArticleModel `gorm:"many2many:article_tags;"`
}
