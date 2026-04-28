package models

import "encoding/json"

type Level int8

const (
	InfoLevel  Level = 1
	WarnLevel  Level = 2
	ErrorLevel Level = 3
)

type LogModel struct {
	ID    uint   `json:"id" gorm:"primaryKey"`
	Title string `gorm:"size:32"`
	Level Level  `json"level"`
}

func (l Level) MarshalJSON() ([]byte, error) {
	var str string
	switch l {
	case InfoLevel:
		str = "info"
	case WarnLevel:
		str = "warn"
	case ErrorLevel:
		str = "error"
	default:
		str = "unknown"
	}
	return json.Marshal(map[string]interface{}{
		"value": int8(l),
		"label": str,
	})
}
