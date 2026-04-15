#include <algorithm>
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

/**
 * @brief         获取当前时间的 Unix 时间戳（毫秒）
 *
 * @return
 */
int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

/**
 * @brief         检查客户端认证信息
 *
 * @param         context
 * @return
 */
grpc::Status CheckAuth(const grpc::ServerContextBase &context) {
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

/**
 * @brief         演示使用回调式 API 实现的 gRPC 服务
 *
 */
class DemoCallbackService final : public DemoService::CallbackService {
public:
    /**
     * @brief         演示一个简单的 Unary
     * RPC，支持模拟处理延迟，并在响应中添加一些元数据
     *
     * @param         context
     * @param         request
     * @param         reply
     * @return
     */
    grpc::ServerUnaryReactor *SayHello(grpc::CallbackServerContext *context,
                                       const HelloRequest *request,
                                       HelloReply *reply) override {
        auto *reactor = context->DefaultReactor();

        const auto auth = CheckAuth(*context);
        if (!auth.ok()) {
            reactor->Finish(auth);
            return reactor;
        }

        if (request->delay_ms() > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(request->delay_ms()));
        }

        reply->set_message("Hello, " + request->name() + " (callback unary)");
        reply->set_server_unix_ms(NowMs());

        context->AddInitialMetadata("x-server", "callback-server");
        context->AddTrailingMetadata("x-processed-by", "callback-server");

        reactor->Finish(grpc::Status::OK);
        return reactor;
    }

    /**
     * @brief         演示一个 Client Streaming
     * RPC，客户端可以上传一系列数字，服务端在客户端完成上传后返回它们的和与数量
     *
     * @param         context
     * @param         reply
     * @return
     */
    grpc::ServerReadReactor<Number> *UploadNumbers(
        grpc::CallbackServerContext *context, SumReply *reply) override {
        return new UploadNumbersReactor(context, reply);
    }

    /**
     * @brief         演示一个 Server Streaming
     *
     * @param         context
     * @param         request
     * @return
     */
    grpc::ServerWriteReactor<CountDownReply> *CountDown(
        grpc::CallbackServerContext *context,
        const CountDownRequest *request) override {
        return new CountDownReactor(context, request);
    }

    /**
     * @brief         演示一个 BiDi Streaming
     * RPC，客户端发送消息后服务端会回复一个包含相同文本的消息
     *
     * @param         context
     * @return
     */
    grpc::ServerBidiReactor<ChatMessage, ChatMessage> *Chat(
        grpc::CallbackServerContext *context) override {
        return new ChatReactor(context);
    }

private:
    /**
     * @brief         UploadNumbers Reactor
     * 的实现类，负责处理客户端上传的一系列数字，并在完成后返回它们的和与数量
     *
     */
    class UploadNumbersReactor final : public grpc::ServerReadReactor<Number> {
    public:
        /**
         * @brief         Construct a new Upload Numbers Reactor object
         *
         * @param         context
         * @param         reply
         */
        UploadNumbersReactor(grpc::CallbackServerContext *context,
                             SumReply *reply)
            : context_(context), reply_(reply) {
            const auto auth = CheckAuth(*context_);
            if (!auth.ok()) {
                Finish(auth);
                return;
            }

            StartRead(&in_);
        }

        /**
         * @brief         当客户端完成上传一个数字后被调用，如果 ok 为
         * false，表示客户端已经完成上传，此时服务端应该计算结果并完成 RPC；如果
         * ok 为 true，表示客户端上传了一个数字，服务端应该将其累加到 sum_
         * 中，并继续等待下一个数字的上传
         *
         * @param         ok
         */
        void OnReadDone(bool ok) override {
            if (!ok) {
                reply_->set_sum(sum_);
                reply_->set_count(count_);
                Finish(grpc::Status::OK);
                return;
            }

            sum_ += in_.value();
            ++count_;
            StartRead(&in_);
        }

        /**
         * @brief         当 RPC
         * 完成时被调用，无论是正常完成还是由于错误完成，都会调用这个方法。在这里我们简单地删除当前的
         * Reactor 实例，因为它是通过 new 创建的，生命周期由 gRPC 框架管理
         *
         */
        void OnDone() override { delete this; }

    private:
        grpc::CallbackServerContext *context_;
        SumReply *reply_;
        Number in_;
        int64_t sum_ = 0;
        uint32_t count_ = 0;
    };

    /**
     * @brief         CountDown Reactor
     * 的实现类，负责处理客户端的倒计时请求，根据请求中的起始数字和间隔时间，定时向客户端发送当前的倒计时数字，直到倒计时结束或客户端取消请求
     *
     */
    class CountDownReactor final
        : public grpc::ServerWriteReactor<CountDownReply> {
    public:
        /**
         * @brief         Construct a new Count Down Reactor object
         *
         * @param         context
         * @param         request
         */
        CountDownReactor(grpc::CallbackServerContext *context,
                         const CountDownRequest *request) {
            const auto auth = CheckAuth(*context);
            if (!auth.ok()) {
                Finish(auth);
                return;
            }

            current_ = std::max(0, request->from());
            interval_ms_ =
                request->interval_ms() == 0 ? 300U : request->interval_ms();
            WriteCurrent();
        }

        /**
         * @brief         当服务端完成向客户端发送一个倒计时数字后被调用，如果
         * ok 为 false，表示客户端已经取消请求，此时服务端应该停止发送并完成
         * RPC；如果 ok 为
         * true，表示服务端成功发送了一个倒计时数字，服务端应该等待一段时间后继续发送下一个数字，直到倒计时结束
         *
         * @param         ok
         */
        void OnWriteDone(bool ok) override {
            if (!ok) {
                Finish(grpc::Status::OK);
                return;
            }

            if (current_ == 0) {
                Finish(grpc::Status::OK);
                return;
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(interval_ms_));
            --current_;
            WriteCurrent();
        }

        void OnDone() override { delete this; }

    private:
        /**
         * @brief         向客户端发送当前的倒计时数字，并在发送完成后继续等待
         * OnWriteDone
         *
         */
        void WriteCurrent() {
            out_.set_current(current_);
            StartWrite(&out_);
        }

        int current_ = 0;
        uint32_t interval_ms_ = 300;
        CountDownReply out_;
    };

    /**
     * @brief         Chat Reactor
     * 的实现类，负责处理客户端的聊天消息，客户端发送一个消息后服务端会回复一个包含相同文本的消息，客户端和服务端可以持续地发送和接收消息，直到任一方完成
     * RPC
     *
     */
    class ChatReactor final
        : public grpc::ServerBidiReactor<ChatMessage, ChatMessage> {
    public:
        /**
         * @brief         Construct a new Chat Reactor object
         *
         * @param         context
         */
        explicit ChatReactor(grpc::CallbackServerContext *context) {
            const auto auth = CheckAuth(*context);
            if (!auth.ok()) {
                Finish(auth);
                return;
            }
            // 首先启动一次读取，异步等待客户端发送第一条消息
            StartRead(&in_);
        }

        void OnReadDone(bool ok) override {
            if (!ok) {
                Finish(grpc::Status::OK);
                return;
            }

            out_.set_user("callback-server");
            out_.set_text("callback-echo<" + in_.user() + ">: " + in_.text());
            out_.set_unix_ms(NowMs());
            StartWrite(&out_);
        }

        void OnWriteDone(bool ok) override {
            if (!ok) {
                Finish(grpc::Status::OK);
                return;
            }

            StartRead(&in_);
        }

        void OnDone() override { delete this; }

    private:
        ChatMessage in_;
        ChatMessage out_;
    };
};

/**
 * @brief         演示使用回调式 API 实现的 gRPC 服务器，监听指定地址，注册
 * DemoCallbackService，并启动服务器等待客户端请求
 *
 */
class CallbackServer final {
public:
    explicit CallbackServer(std::string address)
        : address_(std::move(address)) {}

    void Run() {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        server_ = builder.BuildAndStart();

        std::cout << "[async_callback_server] listening on " << address_
                  << std::endl;
        server_->Wait();
    }

private:
    std::string address_;
    DemoCallbackService service_;
    std::unique_ptr<grpc::Server> server_;
};

} // namespace

int main(int argc, char **argv) {
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50053";
    CallbackServer server(server_addr);
    server.Run();
    return 0;
}
