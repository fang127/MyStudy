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

class AsyncServer final
{
public:
    explicit AsyncServer(std::string address) : address_(std::move(address)) {}

    void Run()
    {
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

        HandleRpcs();
    }

private:
    // Base completion-queue tag interface.
    class ITag
    {
    public:
        virtual ~ITag() = default;
        virtual void OnEvent(bool ok) = 0;
    };

    // 1) Unary async handler: SayHello.
    class AsyncUnaryCall final : public ITag
    {
    public:
        AsyncUnaryCall(DemoService::AsyncService *service,
                       grpc::ServerCompletionQueue *cq)
            : service_(service),
              cq_(cq),
              responder_(&ctx_),
              state_(State::kCreate)
        {
            RequestRpc();
        }

        void OnEvent(bool ok) override
        {
            if (state_ == State::kCreate)
            {
                if (!ok)
                {
                    delete this;
                    return;
                }

                new AsyncUnaryCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok())
                {
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
        enum class State
        {
            kCreate,
            kFinish
        };

        void RequestRpc()
        {
            service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                      this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        HelloRequest request_;
        HelloReply reply_;
        grpc::ServerAsyncResponseWriter<HelloReply> responder_;
        State state_;
    };

    // 2) Client-stream async handler: UploadNumbers.
    class AsyncClientStreamCall final : public ITag
    {
    public:
        AsyncClientStreamCall(DemoService::AsyncService *service,
                              grpc::ServerCompletionQueue *cq)
            : service_(service), cq_(cq), reader_(&ctx_), state_(State::kCreate)
        {
            RequestRpc();
        }

        void OnEvent(bool ok) override
        {
            if (state_ == State::kCreate)
            {
                if (!ok)
                {
                    delete this;
                    return;
                }

                new AsyncClientStreamCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok())
                {
                    state_ = State::kFinish;
                    reader_.Finish(reply_, auth, this);
                    return;
                }

                state_ = State::kRead;
                reader_.Read(&in_, this);
                return;
            }

            if (state_ == State::kRead)
            {
                if (!ok)
                {
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
        enum class State
        {
            kCreate,
            kRead,
            kFinish
        };

        void RequestRpc()
        {
            service_->RequestUploadNumbers(&ctx_, &reader_, cq_, cq_, this);
        }

        DemoService::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        grpc::ServerAsyncReader<SumReply, Number> reader_;
        Number in_;
        SumReply reply_;
        int64_t sum_ = 0;
        uint32_t count_ = 0;
        State state_;
    };

    // 3) Server-stream async handler: CountDown.
    class AsyncServerStreamCall final : public ITag
    {
    public:
        AsyncServerStreamCall(DemoService::AsyncService *service,
                              grpc::ServerCompletionQueue *cq)
            : service_(service), cq_(cq), writer_(&ctx_), state_(State::kCreate)
        {
            RequestRpc();
        }

        void OnEvent(bool ok) override
        {
            if (state_ == State::kCreate)
            {
                if (!ok)
                {
                    delete this;
                    return;
                }

                new AsyncServerStreamCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok())
                {
                    state_ = State::kFinish;
                    writer_.Finish(auth, this);
                    return;
                }

                current_ = request_.from();
                interval_ms_ =
                    request_.interval_ms() == 0 ? 300U : request_.interval_ms();

                state_ = State::kWrite;
                WriteOne();
                return;
            }

            if (state_ == State::kWrite)
            {
                if (!ok)
                {
                    state_ = State::kFinish;
                    writer_.Finish(grpc::Status::OK, this);
                    return;
                }

                if (current_ <= 0)
                {
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
        enum class State
        {
            kCreate,
            kWrite,
            kFinish
        };

        void RequestRpc()
        {
            service_->RequestCountDown(&ctx_, &request_, &writer_, cq_, cq_,
                                       this);
        }

        void WriteOne()
        {
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
    class AsyncBidiChatCall final : public ITag
    {
    public:
        AsyncBidiChatCall(DemoService::AsyncService *service,
                          grpc::ServerCompletionQueue *cq)
            : service_(service), cq_(cq), stream_(&ctx_), state_(State::kCreate)
        {
            RequestRpc();
        }

        void OnEvent(bool ok) override
        {
            if (state_ == State::kCreate)
            {
                if (!ok)
                {
                    delete this;
                    return;
                }

                new AsyncBidiChatCall(service_, cq_);

                const auto auth = CheckAuth(ctx_);
                if (!auth.ok())
                {
                    state_ = State::kFinish;
                    stream_.Finish(auth, this);
                    return;
                }

                state_ = State::kRead;
                stream_.Read(&in_, this);
                return;
            }

            if (state_ == State::kRead)
            {
                if (!ok)
                {
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

            if (state_ == State::kWrite)
            {
                if (!ok)
                {
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
        enum class State
        {
            kCreate,
            kRead,
            kWrite,
            kFinish
        };

        void RequestRpc()
        {
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

    static int64_t NowMs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    void HandleRpcs()
    {
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

int main(int argc, char **argv)
{
    const std::string server_addr = argc > 1 ? argv[1] : "0.0.0.0:50052";
    AsyncServer server(server_addr);
    server.Run();
    return 0;
}
