package utility

import (
	"errors"
	"io"
	"log"
	"os"
)

// 初始化日志系统
func init() {
	// 同时输出到控制台和文件
	f, err := os.OpenFile("app.log", os.O_CREATE|os.O_RDWR|os.O_APPEND, 0644)
	defer f.Close()
	if err != nil {

		panic(errors.New("无法打开日志文件: " + err.Error()))
	}
	multiWriter := io.MultiWriter(os.Stdout, f)
	log.SetOutput(multiWriter)
	log.SetFlags(log.Llongfile | log.Lmicroseconds | log.Ldate)
}
