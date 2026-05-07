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

class AsyncServer final {
public:
    explicit AsyncServer(std::string address) : address_(std::move(address)) {}

    void Run() {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        std::cout << "[async_server] listening on " << address_ << std::endl;

        // Register one CallData instance for each RPC style.
        new AsyncUnaryCall(&service_, cq_.get());
        new AsyncClientStreamCall(&service_, cq_.get());
        new AsyncServerStreamCall(&service_, cq_.get());
        new AsyncBidiChatCall(&service_, cq_.get());

        // 开始处理RPC事件循环，直到服务器关闭。
        HandleRpcs();
    }

private:
    // Base completion-queue tag interface.
    class ITag {
    public:
        virtual ~ITag() = default;
        virtual void OnEvent(bool ok) = 0;
    };

    // 1) Unary async handler: SayHello.
    class AsyncUnaryCall final : public ITag {
    public:
        AsyncUnaryCall(DemoService::AsyncService *service,
                       grpc::ServerCompletionQueue *cq)
            : service_(service),
              cq_(cq),
              responder_(&ctx_),
              state_(State::kCreate) {
            // 对于每个RPC处理器，在构造函数中调用RequestRpc()来请求gRPC
            // runtime通知我们有新的RPC请求到来。
            RequestRpc();
        }

        /**
         * @brief         处理RPC事件的核心方法。当有新的RPC请求到来时，gRPC
         * runtime会调用这个方法，并传递一个布尔值ok，表示事件是否成功发生。根据当前状态（state_），我们可以决定如何处理这个事件。
         *
         * @param         ok
         */
        void OnEvent(bool ok) override {
            if (state_ == State::kCreate) {
                if (!ok) {
                    delete this;
                    return;
                }

                // 对于每个新的RPC请求，我们都创建一个新的CallData实例来处理后续的请求。这确保了服务器能够同时处理多个RPC请求。
                new AsyncUnaryCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                // 在处理RPC请求之前，我们首先检查请求的认证信息。CheckAuth函数会从RPC上下文中提取客户端发送的元数据，验证是否包含正确的令牌（x-token）。如果认证失败，我们直接结束这个RPC调用，并返回相应的gRPC状态码和错误消息；如果认证成功，我们继续处理这个RPC请求。
                if (!auth.ok()) {
                    state_ = State::kFinish;
                    responder_.Finish(reply_, auth, this);
                    return;
                }

                reply_.set_message("Hello, " + request_.name() +
                                   " (async unary)");
                reply_.set_server_unix_ms(NowMs());

                ctx_.AddInitialMetadata("x-server", "async-server");
                ctx_.AddTrailingMetadata("x-processed-by", "async-server");

                state_ = State::kFinish;
                responder_.Finish(reply_, grpc::Status::OK, this);
                return;
            }

            delete this;
        }

    private:
        enum class State { kCreate, kFinish };

        void RequestRpc() {
            // 参数说明：
            // 1. &ctx_:
            // RPC上下文对象，包含RPC调用的相关信息，如元数据、截止时间等。
            // 2. &request_:
            // 用于接收客户端发送的请求消息的HelloRequest对象。
            // 3. &responder_:
            // 用于发送响应消息的ServerAsyncResponseWriter对象。
            // 4. cq_:
            // 用于接收RPC事件的完成队列，服务器通过这个队列来处理RPC事件。
            // 5. cq_: 同上，服务器通过这个队列来处理RPC事件。
            // 6. this: 当前CallData实例的指针，作为事件的标签，gRPC
            // runtime会在事件发生时将这个指针传递给OnEvent方法。
            service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                      this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        HelloRequest request_;
        HelloReply reply_;
        grpc::ServerAsyncResponseWriter<HelloReply>
            responder_; // Unary RPC使用ServerAsyncResponseWriter来发送响应。
        State state_;
    };

    // 2) Client-stream async handler: UploadNumbers.
    class AsyncClientStreamCall final : public ITag {
    public:
        AsyncClientStreamCall(DemoService::AsyncService *service,
                              grpc::ServerCompletionQueue *cq)
            : service_(service),
              cq_(cq),
              reader_(&ctx_),
              state_(State::kCreate) {
            RequestRpc();
        }

        /**
         * @brief         处理RPC事件的核心方法。当有新的RPC请求到来时，gRPC
         * runtime会调用这个方法，并传递一个布尔值ok，表示事件是否成功发生。根据当前状态（state_），我们可以决定如何处理这个事件。
         *
         * @param         ok
         */
        void OnEvent(bool ok) override {
            // 在OnEvent方法中，我们首先检查当前状态。如果状态是kCreate，表示有一个新的RPC请求到来，我们需要为这个请求创建一个新的CallData实例来处理后续的请求。这确保了服务器能够同时处理多个RPC请求。接下来，我们检查认证信息，如果认证失败，我们直接结束这个RPC调用；如果认证成功，我们进入kRead状态，开始读取客户端发送的流式请求。
            if (state_ == State::kCreate) {
                if (!ok) {
                    delete this;
                    return;
                }

                // 对于每个新的RPC请求，我们都创建一个新的CallData实例来处理后续的请求。这确保了服务器能够同时处理多个RPC请求。
                new AsyncClientStreamCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok()) {
                    state_ = State::kFinish;
                    reader_.Finish(reply_, auth, this);
                    return;
                }

                state_ = State::kRead;
                reader_.Read(&in_, this);
                return;
            }

            // 如果状态是kRead，表示我们正在读取客户端发送的流式请求。如果ok为false，表示客户端已经发送完所有数据，我们需要计算结果并结束这个RPC调用；如果ok为true，表示我们成功读取了一条数据，我们将其累加到sum_中，并增加计数器count_，然后继续读取下一条数据。
            if (state_ == State::kRead) {
                if (!ok) {
                    reply_.set_sum(sum_);
                    reply_.set_count(count_);
                    state_ = State::kFinish;
                    reader_.Finish(reply_, grpc::Status::OK, this);
                    return;
                }

                sum_ += in_.value();
                ++count_;
                reader_.Read(&in_, this);
                return;
            }

            delete this;
        }

    private:
        enum class State { kCreate, kRead, kFinish };

        void RequestRpc() {
            // 参数说明：
            // 1. &ctx_:
            // RPC上下文对象，包含RPC调用的相关信息，如元数据、截止时间等。
            // 2. &reader_:
            // 用于接收客户端发送的流式请求的ServerAsyncReader对象。
            // 3. cq_:
            // 用于接收RPC事件的完成队列，服务器通过这个队列来处理RPC事件。
            // 4. cq_: 同上，服务器通过这个队列来处理RPC事件。
            // 5. this: 当前CallData实例的指针，作为事件的标签，gRPC
            // runtime会在事件发生时将这个指针传递给OnEvent方法。
            service_->RequestUploadNumbers(&ctx_, &reader_, cq_, cq_, this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        grpc::ServerAsyncReader<SumReply, Number>
            reader_; // Client-stream
                     // RPC使用ServerAsyncReader来接收客户端发送的流式请求。
        Number in_;
        SumReply reply_;
        int64_t sum_ = 0;
        uint32_t count_ = 0;
        State state_;
    };

    // 3) Server-stream async handler: CountDown.
    class AsyncServerStreamCall final : public ITag {
    public:
        AsyncServerStreamCall(DemoService::AsyncService *service,
                              grpc::ServerCompletionQueue *cq)
            : service_(service),
              cq_(cq),
              writer_(&ctx_),
              state_(State::kCreate) {
            RequestRpc();
        }

        /**
         * @brief         处理RPC事件的核心方法。当有新的RPC请求到来时，gRPC
         * runtime会调用这个方法，并传递一个布尔值ok，表示事件是否成功发生。根据当前状态（state_），我们可以决定如何处理这个事件。
         *
         * @param         ok
         */
        void OnEvent(bool ok) override {
            // 在OnEvent方法中，我们首先检查当前状态。如果状态是kCreate，表示有一个新的RPC请求到来，我们需要为这个请求创建一个新的CallData实例来处理后续的请求。这确保了服务器能够同时处理多个RPC请求。接下来，我们检查认证信息，如果认证失败，我们直接结束这个RPC调用；如果认证成功，我们设置当前的计数值和发送间隔，然后进入kWrite状态，开始向客户端发送流式响应。
            if (state_ == State::kCreate) {
                if (!ok) {
                    delete this;
                    return;
                }

                new AsyncServerStreamCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok()) {
                    state_ = State::kFinish;
                    writer_.Finish(auth, this);
                    return;
                }

                // 根据客户端请求中的from字段设置当前的计数值（current_），并根据interval_ms字段设置发送间隔（interval_ms_）。如果客户端没有指定发送间隔，我们默认使用300毫秒。然后我们进入kWrite状态，开始向客户端发送流式响应。
                current_ = request_.from();
                interval_ms_ =
                    request_.interval_ms() == 0 ? 300U : request_.interval_ms();

                state_ = State::kWrite;
                WriteOne();
                return;
            }

            // 如果状态是kWrite，表示我们正在向客户端发送流式响应。如果ok为false，表示客户端已经关闭连接，我们需要结束这个RPC调用；如果ok为true，表示我们成功发送了一条数据，我们检查当前的计数值是否已经小于等于0，如果是，我们结束这个RPC调用；如果不是，我们继续发送下一条数据。在发送下一条数据之前，我们还进行了一些Demo友好的节流处理，让这个流在日志中看起来更明显。
            if (state_ == State::kWrite) {
                if (!ok) {
                    state_ = State::kFinish;
                    writer_.Finish(grpc::Status::OK, this);
                    return;
                }

                if (current_ <= 0) {
                    state_ = State::kFinish;
                    writer_.Finish(grpc::Status::OK, this);
                    return;
                }

                // Demo-friendly throttling so the stream looks visible in logs.
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(interval_ms_));
                --current_;
                WriteOne();
                return;
            }

            delete this;
        }

    private:
        enum class State { kCreate, kWrite, kFinish };

        void RequestRpc() {
            service_->RequestCountDown(&ctx_, &request_, &writer_, cq_, cq_,
                                       this);
        }

        /**
         * @brief
         * 这个方法负责向客户端发送一个CountDownReply消息。它首先设置当前的计数值（current_）到响应消息中，然后调用writer_.Write()方法将这个消息发送给客户端。Write()方法是异步的，当消息成功发送后，gRPC
         * runtime会再次调用OnEvent方法，并将ok参数设置为true，这时我们可以继续处理下一条消息或者结束这个RPC调用。
         *
         */
        void WriteOne() {
            out_.set_current(current_);
            writer_.Write(out_, this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        CountDownRequest request_;
        CountDownReply out_;
        grpc::ServerAsyncWriter<CountDownReply> writer_;
        int current_ = 0;
        uint32_t interval_ms_ = 300;
        State state_;
    };

    // 4) Bidirectional-stream async handler: Chat.
    class AsyncBidiChatCall final : public ITag {
    public:
        AsyncBidiChatCall(DemoService::AsyncService *service,
                          grpc::ServerCompletionQueue *cq)
            : service_(service),
              cq_(cq),
              stream_(&ctx_),
              state_(State::kCreate) {
            RequestRpc();
        }

        void OnEvent(bool ok) override {
            // 在OnEvent方法中，我们首先检查当前状态。如果状态是kCreate，表示有一个新的RPC请求到来，我们需要为这个请求创建一个新的CallData实例来处理后续的请求。这确保了服务器能够同时处理多个RPC请求。接下来，我们检查认证信息，如果认证失败，我们直接结束这个RPC调用；如果认证成功，我们进入kRead状态，开始读取客户端发送的流式请求。
            if (state_ == State::kCreate) {
                if (!ok) {
                    delete this;
                    return;
                }

                new AsyncBidiChatCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok()) {
                    state_ = State::kFinish;
                    stream_.Finish(auth, this);
                    return;
                }

                state_ = State::kRead;
                stream_.Read(&in_, this);
                return;
            }

            // 如果状态是kRead，表示我们正在读取客户端发送的流式请求。如果ok为false，表示客户端已经关闭连接，我们需要结束这个RPC调用；如果ok为true，表示我们成功读取了一条数据，我们根据这条数据生成一个响应消息，然后进入kWrite状态，开始向客户端发送流式响应。
            if (state_ == State::kRead) {
                if (!ok) {
                    state_ = State::kFinish;
                    stream_.Finish(grpc::Status::OK, this);
                    return;
                }

                out_.set_user("async-server");
                out_.set_text("async-echo<" + in_.user() + ">: " + in_.text());
                out_.set_unix_ms(NowMs());

                state_ = State::kWrite;
                stream_.Write(out_, this);
                return;
            }

            // 如果状态是kWrite，表示我们正在向客户端发送流式响应。如果ok为false，表示客户端已经关闭连接，我们需要结束这个RPC调用；如果ok为true，表示我们成功发送了一条数据，我们继续进入kRead状态，等待客户端发送下一条数据。在这个双向流式RPC中，服务器和客户端可以同时发送和接收消息，因此我们在发送完一条响应消息后，继续等待客户端发送下一条请求消息。
            if (state_ == State::kWrite) {
                if (!ok) {
                    state_ = State::kFinish;
                    stream_.Finish(grpc::Status::OK, this);
                    return;
                }

                state_ = State::kRead;
                stream_.Read(&in_, this);
                return;
            }

            delete this;
        }

    private:
        enum class State { kCreate, kRead, kWrite, kFinish };

        void RequestRpc() {
            service_->RequestChat(&ctx_, &stream_, cq_, cq_, this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        grpc::ServerAsyncReaderWriter<ChatMessage, ChatMessage> stream_;
        ChatMessage in_;
        ChatMessage out_;
        State state_;
    };

    /**
     * @brief
     * 检查RPC请求的认证信息。它从RPC上下文中提取客户端发送的元数据，验证是否包含正确的令牌（x-token）。如果认证失败，返回相应的gRPC状态码和错误消息；如果认证成功，返回OK状态。
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

    /**
     * @brief
     * 获取当前时间的Unix时间戳（以毫秒为单位）。它使用C++11的chrono库来获取系统当前时间，并将其转换为自1970年1月1日以来的毫秒数。这在生成响应消息时用于记录服务器处理请求的时间戳。
     *
     * @return
     */
    static int64_t NowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    /**
     * @brief
     * 处理RPC事件循环，直到服务器关闭。每当有新的RPC请求到来时，相关的CallData实例会被触发并处理请求。
     *
     */
    void HandleRpcs() {
        void *tag = nullptr;
        bool ok = false;
        while (cq_->Next(&tag, &ok)) static_cast<ITag *>(tag)->OnEvent(ok);
    }

    std::string address_;
    DemoService::AsyncService service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::unique_ptr<grpc::Server> server_;
};

} // namespace

int main(int argc, char **argv) {
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50052";
    AsyncServer server(server_addr);
    server.Run();
    return 0;
}
