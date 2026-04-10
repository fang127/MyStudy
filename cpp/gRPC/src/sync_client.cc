#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "demo.grpc.pb.h"

namespace
{

using tutorial::grpcdemo::ChatMessage;
using tutorial::grpcdemo::CountDownReply;
using tutorial::grpcdemo::CountDownRequest;
using tutorial::grpcdemo::DemoService;
using tutorial::grpcdemo::HelloReply;
using tutorial::grpcdemo::HelloRequest;
using tutorial::grpcdemo::Number;
using tutorial::grpcdemo::SumReply;

constexpr const char *kToken = "demo-token";

class DemoClient
{
public:
    explicit DemoClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel)))
    {
    }

    void RunAll()
    {
        DoUnary();
        DoClientStreaming();
        DoServerStreaming();
        DoBidiStreaming();
    }

private:
    void FillContext(grpc::ClientContext *context, int deadline_ms)
    {
        // 增加一些元信息和超时设置，演示客户端功能
        context->AddMetadata("x-token", kToken);            // 认证信息
        context->AddMetadata("x-client-id", "sync-client"); // 其他信息
        context->set_deadline(
            std::chrono::system_clock::now() +
            std::chrono::milliseconds(deadline_ms)); // 设置 RPC 超时
        // 表示“服务暂时不可用时先等连接就绪再发”，而不是立刻失败。设置等待服务端准备好再发起
        // RPC 调用，适用于服务端启动较慢的场景
        context->set_wait_for_ready(true);
    }

    void DoUnary()
    {
        grpc::ClientContext context;
        FillContext(&context, 4000);

        HelloRequest req;
        req.set_name("grpc learner");
        req.set_delay_ms(150);

        HelloReply rep;
        grpc::Status status = stub_->SayHello(&context, req, &rep);

        std::cout << "\n[Unary] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
        if (status.ok())
        {
            std::cout << "[Unary] reply: " << rep.message()
                      << ", server_unix_ms=" << rep.server_unix_ms()
                      << std::endl;

            for (const auto &[k, v] : context.GetServerInitialMetadata())
            {
                std::cout << "[Unary] initial md: "
                          << std::string(k.data(), k.length()) << "="
                          << std::string(v.data(), v.length()) << std::endl;
            }
            for (const auto &[k, v] : context.GetServerTrailingMetadata())
            {
                std::cout << "[Unary] trailing md: "
                          << std::string(k.data(), k.length()) << "="
                          << std::string(v.data(), v.length()) << std::endl;
            }
        }
    }

    void DoClientStreaming()
    {
        grpc::ClientContext context;
        FillContext(&context, 4000);

        SumReply rep;
        std::unique_ptr<grpc::ClientWriter<Number>> writer(
            stub_->UploadNumbers(&context, &rep));

        for (int i = 1; i <= 5; ++i)
        {
            Number n;
            n.set_value(i);
            if (!writer->Write(n)) break;
        }

        writer->WritesDone();
        grpc::Status status = writer->Finish();

        std::cout << "\n[ClientStream] status=" << status.error_code() << " "
                  << status.error_message() << std::endl;
        if (status.ok())
        {
            std::cout << "[ClientStream] sum=" << rep.sum()
                      << ", count=" << rep.count() << std::endl;
        }
    }

    void DoServerStreaming()
    {
        grpc::ClientContext context;
        FillContext(&context, 6000);

        CountDownRequest req;
        req.set_from(5);
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

    void DoBidiStreaming()
    {
        grpc::ClientContext context;
        FillContext(&context, 10000);

        std::shared_ptr<grpc::ClientReaderWriter<ChatMessage, ChatMessage>>
            stream(stub_->Chat(&context));

        std::thread writer(
            [stream]()
            {
                for (int i = 0; i < 3; ++i)
                {
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
        while (stream->Read(&resp))
        {
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

int main(int argc, char **argv)
{
    const std::string target = argc > 1 ? argv[1] : "127.0.0.1:50051";

    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 20 * 1000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5 * 1000);
    args.SetInt(GRPC_ARG_HTTP2_BDP_PROBE, 1);
    args.SetMaxReceiveMessageSize(16 * 1024 * 1024);

    auto channel = grpc::CreateCustomChannel(
        target, grpc::InsecureChannelCredentials(), args);

    DemoClient client(channel);
    client.RunAll();
    return 0;
}
