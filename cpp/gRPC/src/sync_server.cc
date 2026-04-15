#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "demo.grpc.pb.h"

namespace {

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

class DemoServiceImpl final : public DemoService::Service {
public:
    /**
     * @brief
     * 一个简单的同步单向RPC方法实现，处理客户端的Hello请求并返回问候语和服务器时间戳
     *
     * @param         context
     * @param         request
     * @param         reply
     * @return
     */
    grpc::Status SayHello(grpc::ServerContext *context,
                          const HelloRequest *request,
                          HelloReply *reply) override {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        context->AddInitialMetadata("x-server", "sync-server");

        if (request->delay_ms() > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(request->delay_ms()));
        }

        // 在处理请求的过程中，定期检查是否已取消，以便及时响应取消请求
        // 比如：客户端设置了context的deadline，或者客户端主动取消了请求，都可能导致服务器端的context被标记为已取消
        if (context->IsCancelled()) {
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

    /**
     * @brief         客户端流式 RPC
     *
     * @param         context
     * @param         reader
     * @param         reply
     * @return
     */
    grpc::Status UploadNumbers(grpc::ServerContext *context,
                               grpc::ServerReader<Number> *reader,
                               SumReply *reply) override {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        int64_t sum = 0;
        uint32_t count = 0;
        Number n;
        while (reader->Read(&n)) {
            sum += n.value();
            ++count;
            if (context->IsCancelled()) {
                return grpc::Status(grpc::StatusCode::CANCELLED,
                                    "upload cancelled");
            }
        }

        reply->set_sum(sum);
        reply->set_count(count);
        context->AddTrailingMetadata("x-sum-source", "client-stream");
        return grpc::Status::OK;
    }

    /**
     * @brief         服务器流式 RPC
     *
     * @param         context
     * @param         request
     * @param         writer
     * @return
     */
    grpc::Status CountDown(
        grpc::ServerContext *context, const CountDownRequest *request,
        grpc::ServerWriter<CountDownReply> *writer) override {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        const uint32_t interval =
            request->interval_ms() == 0 ? 300U : request->interval_ms();

        for (int i = request->from(); i >= 0; --i) {
            if (context->IsCancelled()) {
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

    /**
     * @brief         双向流式 RPC
     *
     * @param         context
     * @param         stream
     * @return
     */
    grpc::Status Chat(
        grpc::ServerContext *context,
        grpc::ServerReaderWriter<ChatMessage, ChatMessage> *stream) override {
        auto auth = CheckAuth(*context);
        if (!auth.ok()) return auth;

        ChatMessage in;
        // 服务器端在处理双向流式RPC时，通常会在一个循环中持续读取客户端发送的消息，并根据需要发送响应。在这个过程中，定期检查context是否已取消是非常重要的，以便能够及时响应取消请求，避免不必要的资源消耗和延迟。
        // 例如：如果客户端设置了context的deadline，或者客户端主动取消了请求，都可能导致服务器端的context被标记为已取消。在这种情况下，服务器端应该尽快停止处理当前的消息，并返回一个适当的状态码（如CANCELLED）给客户端，以通知客户端请求已被取消。
        // 也可以读一次，写多次，或者读多次，写一次，甚至读多次，写多次，这取决于具体的业务需求和设计。
        while (stream->Read(&in)) {
            if (context->IsCancelled()) {
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
    /**
     * @brief
     * 一个简单的认证检查函数，验证客户端请求中是否包含正确的x-token元数据
     *
     * @param         context
     * @return
     */
    static grpc::Status CheckAuth(const grpc::ServerContext &context) {
        const auto &md = context.client_metadata();
        const auto it = md.find(kTokenKey);
        if (it == md.end()) {
            return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                                "missing x-token metadata");
        }

        const std::string token(it->second.data(), it->second.length());
        if (token != kTokenValue) {
            return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                                "invalid x-token value");
        }

        return grpc::Status::OK;
    }
};

} // namespace

int main(int argc, char **argv) {
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50051";

    DemoServiceImpl service;
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 设置最大接收消息大小为16MB，默认为4MB
    builder.SetMaxReceiveMessageSize(16 * 1024 * 1024);
    // 设置最大发送消息大小为16MB，默认为4MB
    builder.SetMaxSendMessageSize(16 * 1024 * 1024);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "[sync_server] listening on " << server_addr << std::endl;
    server->Wait();
    return 0;
}
