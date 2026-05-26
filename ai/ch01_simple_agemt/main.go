package main

import (
	"ai/utility"
	"context"
	"log"

	"github.com/cloudwego/eino-ext/components/model/ollama"
	"github.com/cloudwego/eino/schema"
)

func CreateChatAgent(ctx context.Context, config utility.Config) (*ollama.ChatModel, error) {
	agent, err := ollama.NewChatModel(ctx, &ollama.ChatModelConfig{
		Model:   config.ModelName,
		BaseURL: config.ModelUrl,
	})
	if err != nil {
		return nil, err
	}
	return agent, err
}

func main() {
	context := context.Background()
	config, err := utility.Load()
	if err != nil {
		log.Fatal("环境变量加载失败: ", err)
	}
	agent, err := CreateChatAgent(context, config)
	if err != nil {
		log.Fatal("创建Agent失败: ", err)
	}

	resp, err := agent.Generate(context, []*schema.Message{
		{
			Role:    schema.User,
			Content: "请介绍一下人工智能的历史。",
		},
	})
	if err != nil {
		log.Fatal("生成响应失败: ", err)
	}
	log.Printf("Agent响应: \n%v\n", resp)
}
