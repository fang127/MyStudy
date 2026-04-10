#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

constexpr const char *kTokenKey = "x-token";
constexpr const char *kTokenValue = "demo-token";

class DemoServiceImpl final : public DemoService::Service
{
public:
    grpc::Status SayHello(grpc::ServerContext *context,
                          const HelloRequest *request,
                          HelloReply *reply) override
    {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        context->AddInitialMetadata("x-server", "sync-server");

        if (request->delay_ms() > 0)
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(request->delay_ms()));
        }

        if (context->IsCancelled())
        {
            return grpc::Status(grpc::StatusCode::CANCELLED,
                                "request cancelled by client");
        }

        reply->set_message("Hello, " + request->name() + " (sync unary)");
        const auto now_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
        reply->set_server_unix_ms(now_ms);

        context->AddTrailingMetadata("x-processed-by", "sync-server");
        return grpc::Status::OK;
    }

    grpc::Status UploadNumbers(grpc::ServerContext *context,
                               grpc::ServerReader<Number> *reader,
                               SumReply *reply) override
    {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        int64_t sum = 0;
        uint32_t count = 0;
        Number n;
        while (reader->Read(&n))
        {
            sum += n.value();
            ++count;
            if (context->IsCancelled())
            {
                return grpc::Status(grpc::StatusCode::CANCELLED,
                                    "upload cancelled");
            }
        }

        reply->set_sum(sum);
        reply->set_count(count);
        context->AddTrailingMetadata("x-sum-source", "client-stream");
        return grpc::Status::OK;
    }

    grpc::Status CountDown(grpc::ServerContext *context,
                           const CountDownRequest *request,
                           grpc::ServerWriter<CountDownReply> *writer) override
    {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        const uint32_t interval =
            request->interval_ms() == 0 ? 300U : request->interval_ms();

        for (int i = request->from(); i >= 0; --i)
        {
            if (context->IsCancelled())
            {
                return grpc::Status(grpc::StatusCode::CANCELLED,
                                    "countdown cancelled");
            }
            CountDownReply out;
            out.set_current(i);
            if (!writer->Write(out)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

        return grpc::Status::OK;
    }

    grpc::Status Chat(
        grpc::ServerContext *context,
        grpc::ServerReaderWriter<ChatMessage, ChatMessage> *stream) override
    {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        ChatMessage in;
        while (stream->Read(&in))
        {
            if (context->IsCancelled())
            {
                return grpc::Status(grpc::StatusCode::CANCELLED,
                                    "chat cancelled");
            }

            ChatMessage out;
            out.set_user("server");
            out.set_text("echo<" + in.user() + ">: " + in.text());
            const auto now_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
            out.set_unix_ms(now_ms);

            if (!stream->Write(out)) break;
        }

        return grpc::Status::OK;
    }

private:
    static grpc::Status CheckAuth(const grpc::ServerContext &context)
    {
        const auto &md = context.client_metadata();
        const auto it = md.find(kTokenKey);
        if (it == md.end())
        {
            return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                                "missing x-token metadata");
        }

        const std::string token(it->second.data(), it->second.length());
        if (token != kTokenValue)
        {
            return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                                "invalid x-token value");
        }

        return grpc::Status::OK;
    }
};

} // namespace

int main(int argc, char **argv)
{
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50051";

    DemoServiceImpl service;
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    builder.SetMaxReceiveMessageSize(16 * 1024 * 1024);
    builder.SetMaxSendMessageSize(16 * 1024 * 1024);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "[sync_server] listening on " << server_addr << std::endl;
    server->Wait();
    return 0;
}
