package utility

import (
	"io"
	"log"
	"os"
)

// 初始化日志系统
func InitLogger(f io.Writer) {
	// 同时输出到控制台和文件
	multiWriter := io.MultiWriter(os.Stdout, f)
	log.SetOutput(multiWriter)
	log.SetFlags(log.Llongfile | log.Lmicroseconds | log.Ldate)
}
