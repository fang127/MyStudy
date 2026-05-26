package utility

import "os"

type Config struct {
	ModelName   string
	ModelUrl    string
	ModelApiKey string
}

// 加载环境变量
func Load() (Config, error) {
	config := Config{
		ModelName:   GetEnv("MODEL_NAME", "llama3.1"),
		ModelUrl:    GetEnv("MODEL_URL", "http://localhost:11434"),
		ModelApiKey: GetEnv("MODEL_API_KEY", ""),
	}
	return config, nil
}

// GetEnv 获取环境变量，如果不存在则返回默认值
func GetEnv(key, defaultValue string) string {
	value := os.Getenv(key)
	if value == "" {
		return defaultValue
	}
	return value
}
