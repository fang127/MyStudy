#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "demo.grpc.pb.h"

namespace {

using tutorial::grpcdemo::ChatMessage;      // 双向流的消息类型
using tutorial::grpcdemo::CountDownReply;   // 服务端流的消息类型
using tutorial::grpcdemo::CountDownRequest; // 服务端流的请求类型
using tutorial::grpcdemo::DemoService;      // gRPC 服务定义
using tutorial::grpcdemo::HelloReply;       // 普通 RPC 的回复类型
using tutorial::grpcdemo::HelloRequest;     // 普通 RPC 的请求类型
using tutorial::grpcdemo::Number;           // 客户端流的消息类型
using tutorial::grpcdemo::SumReply;         // 客户端流的回复类型

constexpr const char *kToken = "demo-token";

class DemoClient {
public:
    explicit DemoClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    void RunAll() {
        DoUnary();
        DoClientStreaming();
        DoServerStreaming();
        DoBidiStreaming();
    }

private:
    /**
     * @brief         填充 gRPC 客户端上下文，添加元信息和设置超时等选项
     *
     * @param         context
     * @param         deadline_ms
     */
    void FillContext(grpc::ClientContext *context, int deadline_ms) {
        // 增加一些元信息和超时设置，演示客户端功能
        context->AddMetadata("x-token", kToken);            // 认证信息
        context->AddMetadata("x-client-id", "sync-client"); // 其他信息
        // 设置 RPC
        // 调用的截止时间，单位为毫秒。客户端会在这个时间点之后放弃等待服务器的回复，返回一个超时错误
        // 本质上是告诉 gRPC 库在发起 RPC
        // 调用时设置一个超时时间，超过这个时间如果还没有收到服务器的回复，就认为调用失败并返回错误。这样可以避免客户端无限期地等待服务器的响应，提高系统的健壮性和用户体验。
        context->set_deadline(
            std::chrono::system_clock::now() +
            std::chrono::milliseconds(deadline_ms)); // 设置 RPC 超时
        // 表示“服务暂时不可用时先等连接就绪再发”，而不是立刻失败。设置等待服务端准备好再发起
        // RPC 调用，适用于服务端启动较慢的场景
        context->set_wait_for_ready(true);
    }

    /**
     * @brief         演示普通的 Unary RPC
     * 调用，发送一个请求并等待回复，同时展示状态码、错误信息和服务器返回的元信息
     *
     */
    void DoUnary() {
        grpc::ClientContext context;
        FillContext(&context, 4000);

        HelloRequest req;
        req.set_name("grpc learner");
        req.set_delay_ms(150);

        HelloReply rep;
        grpc::Status status = stub_->SayHello(&context, req, &rep);

        std::cout << "\n[Unary] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
        if (status.ok()) {
            std::cout << "[Unary] reply: " << rep.message()
                      << ", server_unix_ms=" << rep.server_unix_ms()
                      << std::endl;

            for (const auto &[k, v] : context.GetServerInitialMetadata()) {
                std::cout << "[Unary] initial md: "
                          << std::string(k.data(), k.length()) << "="
                          << std::string(v.data(), v.length()) << std::endl;
            }
            for (const auto &[k, v] : context.GetServerTrailingMetadata()) {
                std::cout << "[Unary] trailing md: "
                          << std::string(k.data(), k.length()) << "="
                          << std::string(v.data(), v.length()) << std::endl;
            }
        }
    }

    /**
     * @brief         演示客户端流式
     * RPC，客户端连续发送多个消息，最后等待服务器回复一个结果
     *
     */
    void DoClientStreaming() {
        grpc::ClientContext context;
        FillContext(&context, 4000);

        SumReply rep;
        // 创建一个客户端流，准备发送多个 Number 消息，并最终接收一个 SumReply
        // 结果的状态通过 Finish() 获取，Write() 返回 false
        // 代表流已关闭无法继续写入
        std::unique_ptr<grpc::ClientWriter<Number>> writer(
            stub_->UploadNumbers(&context, &rep));

        for (int i = 1; i <= 5; ++i) {
            Number n;
            n.set_value(i);
            if (!writer->Write(n)) break;
        }

        writer->WritesDone();
        grpc::Status status = writer->Finish();

        std::cout << "\n[ClientStream] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
        if (status.ok()) {
            std::cout << "[ClientStream] sum=" << rep.sum()
                      << ", count=" << rep.count() << std::endl;
        }
    }

    /**
     * @brief         演示服务端流式
     * RPC，客户端发送一个请求，服务器连续发送多个消息作为回复
     *
     */
    void DoServerStreaming() {
        grpc::ClientContext context;
        FillContext(&context, 6000);

        CountDownRequest req;
        req.set_from(5);
        // 120ms
        // 的间隔演示服务器分多次回复，客户端每次收到后可以处理一下再接收下一条
        req.set_interval_ms(120);

        std::unique_ptr<grpc::ClientReader<CountDownReply>> reader(
            stub_->CountDown(&context, req));

        std::cout << "\n[ServerStream] receiving:" << std::endl;
        CountDownReply item;
        while (reader->Read(&item))
            std::cout << "  " << item.current() << std::endl;

        grpc::Status status = reader->Finish();
        std::cout << "[ServerStream] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
    }

    /**
     * @brief         演示双向流式
     *
     */
    void DoBidiStreaming() {
        grpc::ClientContext context;
        FillContext(&context, 10000);

        std::shared_ptr<grpc::ClientReaderWriter<ChatMessage, ChatMessage>>
            stream(stub_->Chat(&context));

        std::thread writer([stream]() {
            for (int i = 0; i < 3; ++i) {
                ChatMessage msg;
                msg.set_user("client");
                msg.set_text("message-" + std::to_string(i));
                const auto now_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
                msg.set_unix_ms(now_ms);
                if (!stream->Write(msg)) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            stream->WritesDone();
        });

        std::cout << "\n[Bidi] replies:" << std::endl;
        ChatMessage resp;
        // 客户端和服务器可以同时发送和接收消息，互不阻塞。客户端在一个线程里发送消息，在另一个线程里接收服务器的回复。
        // 服务器每收到一个消息就回复一个消息，客户端可以边发边收，演示双向流的特性
        // 客户端在发送完所有消息并调用 WritesDone()
        // 后，仍然可以继续接收服务器的回复，直到服务器关闭流或发生错误
        while (stream->Read(&resp)) {
            std::cout << "  " << resp.user() << ": " << resp.text() << " @"
                      << resp.unix_ms() << std::endl;
        }

        writer.join();
        grpc::Status status = stream->Finish();
        std::cout << "[Bidi] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
    }

    std::unique_ptr<DemoService::Stub> stub_;
};

} // namespace

int main(int argc, char **argv) {
    const std::string target = argc > 1 ? argv[1] : "127.0.0.1:50051";

    grpc::ChannelArguments args;
    // 20s 没有数据就发探测包，保持连接活跃
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 20 * 1000);
    // 5s 没有收到探测包回复就认为连接不可用
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5 * 1000);
    // 启用带宽延迟积探测，自动调整 TCP 窗口大小以适应网络状况
    args.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);
    args.SetMaxReceiveMessageSize(16 * 1024 * 1024); // 16MB

    auto channel = grpc::CreateCustomChannel(
        target, grpc::InsecureChannelCredentials(), args);

    DemoClient client(channel);
    client.RunAll();
    return 0;
}
