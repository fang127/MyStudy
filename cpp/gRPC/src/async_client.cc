#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

// -----------------------------
// Async unary demo client
// -----------------------------
struct AsyncUnaryCall {
    HelloReply reply;
    grpc::ClientContext context;
    grpc::Status status;
    std::unique_ptr<grpc::ClientAsyncResponseReader<HelloReply>> responder;
};

class AsyncUnaryClient {
public:
    explicit AsyncUnaryClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    void SendHello(const std::string &name, int delay_ms) {
        auto *call = new AsyncUnaryCall;
        FillCommonMetadata(&call->context, "async-unary-client", 5000);

        HelloRequest request;
        request.set_name(name);
        request.set_delay_ms(delay_ms);

        call->responder = stub_->AsyncSayHello(&call->context, request, &cq_);
        call->responder->Finish(&call->reply, &call->status,
                                static_cast<void *>(call));
    }

    void DrainResponses(int expected) {
        int done = 0;
        while (done < expected) {
            void *tag = nullptr;
            bool ok = false;
            if (!cq_.Next(&tag, &ok)) break;

            auto *call = static_cast<AsyncUnaryCall *>(tag);
            if (ok && call->status.ok()) {
                std::cout << "[AsyncUnary] " << call->reply.message()
                          << ", server_unix_ms=" << call->reply.server_unix_ms()
                          << std::endl;
            } else {
                std::cout << "[AsyncUnary] failed: "
                          << call->status.error_code() << " "
                          << call->status.error_message() << std::endl;
            }

            delete call;
            ++done;
        }
    }

private:
    static void FillCommonMetadata(grpc::ClientContext *context,
                                   const std::string &client_id,
                                   int deadline_ms) {
        context->AddMetadata("x-token", "demo-token");
        context->AddMetadata("x-client-id", client_id);
        context->set_deadline(std::chrono::system_clock::now() +
                              std::chrono::milliseconds(deadline_ms));
    }

    std::unique_ptr<DemoService::Stub> stub_;
    grpc::CompletionQueue cq_;
};

// --------------------------------------
// Async client-stream demo client
// UploadNumbers(stream Number) -> SumReply
// --------------------------------------
class AsyncClientStreamClient {
public:
    explicit AsyncClientStreamClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    int RunDemo() {
        FillCommonMetadata(&context_, "async-client-stream-client", 8000);

        writer_ = stub_->PrepareAsyncUploadNumbers(&context_, &reply_, &cq_);
        writer_->StartCall(Tag(TagType::kStartCall));

        values_ = {1, 2, 3, 4, 5};

        while (true) {
            void *tag = nullptr;
            bool ok = false;
            if (!cq_.Next(&tag, &ok)) {
                std::cerr << "[AsyncClientStream] completion queue closed "
                             "unexpectedly"
                          << std::endl;
                return 1;
            }

            const TagType type = FromTag(tag);

            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] StartCall failed"
                              << std::endl;
                    return 1;
                }
                WriteNext();
                continue;
            }

            if (type == TagType::kWrite) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] Write failed"
                              << std::endl;
                    return 1;
                }

                if (index_ < values_.size())
                    WriteNext();
                else
                    writer_->WritesDone(Tag(TagType::kWritesDone));
                continue;
            }

            if (type == TagType::kWritesDone) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] WritesDone failed"
                              << std::endl;
                    return 1;
                }
                writer_->Finish(&status_, Tag(TagType::kFinish));
                continue;
            }

            if (type == TagType::kFinish) {
                if (!ok || !status_.ok()) {
                    std::cout << "[AsyncClientStream] failed: "
                              << status_.error_code() << " "
                              << status_.error_message() << std::endl;
                    return 1;
                }

                std::cout << "[AsyncClientStream] sum=" << reply_.sum()
                          << ", count=" << reply_.count() << std::endl;
                return 0;
            }
        }
    }

private:
    enum class TagType : std::intptr_t {
        kStartCall = 1,
        kWrite = 2,
        kWritesDone = 3,
        kFinish = 4,
    };

    static void *Tag(TagType t) {
        return reinterpret_cast<void *>(static_cast<std::intptr_t>(t));
    }

    static TagType FromTag(void *p) {
        return static_cast<TagType>(reinterpret_cast<std::intptr_t>(p));
    }

    static void FillCommonMetadata(grpc::ClientContext *context,
                                   const std::string &client_id,
                                   int deadline_ms) {
        context->AddMetadata("x-token", "demo-token");
        context->AddMetadata("x-client-id", client_id);
        context->set_deadline(std::chrono::system_clock::now() +
                              std::chrono::milliseconds(deadline_ms));
    }

    void WriteNext() {
        write_msg_.set_value(values_.at(index_));
        ++index_;
        writer_->Write(write_msg_, Tag(TagType::kWrite));
    }

    std::unique_ptr<DemoService::Stub> stub_;
    grpc::CompletionQueue cq_;
    grpc::ClientContext context_;
    grpc::Status status_;
    SumReply reply_;
    Number write_msg_;
    std::vector<int> values_;
    std::size_t index_ = 0;
    std::unique_ptr<grpc::ClientAsyncWriter<Number>> writer_;
};

// --------------------------------------
// Async server-stream demo client
// CountDown(request) -> stream CountDownReply
// --------------------------------------
class AsyncServerStreamClient {
public:
    explicit AsyncServerStreamClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    int RunDemo() {
        FillCommonMetadata(&context_, "async-server-stream-client", 8000);

        request_.set_from(5);
        request_.set_interval_ms(120);

        reader_ = stub_->PrepareAsyncCountDown(&context_, request_, &cq_);
        reader_->StartCall(Tag(TagType::kStartCall));

        while (true) {
            void *tag = nullptr;
            bool ok = false;
            if (!cq_.Next(&tag, &ok)) {
                std::cerr << "[AsyncServerStream] completion queue closed "
                             "unexpectedly"
                          << std::endl;
                return 1;
            }

            const TagType type = FromTag(tag);

            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncServerStream] StartCall failed"
                              << std::endl;
                    return 1;
                }
                reader_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            if (type == TagType::kRead) {
                if (!ok) {
                    reader_->Finish(&status_, Tag(TagType::kFinish));
                    continue;
                }

                std::cout << "[AsyncServerStream] current="
                          << read_msg_.current() << std::endl;
                reader_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            if (type == TagType::kFinish) {
                if (!status_.ok()) {
                    std::cout << "[AsyncServerStream] failed: "
                              << status_.error_code() << " "
                              << status_.error_message() << std::endl;
                    return 1;
                }

                std::cout << "[AsyncServerStream] finished OK" << std::endl;
                return 0;
            }
        }
    }

private:
    enum class TagType : std::intptr_t {
        kStartCall = 1,
        kRead = 2,
        kFinish = 3,
    };

    static void *Tag(TagType t) {
        return reinterpret_cast<void *>(static_cast<std::intptr_t>(t));
    }

    static TagType FromTag(void *p) {
        return static_cast<TagType>(reinterpret_cast<std::intptr_t>(p));
    }

    static void FillCommonMetadata(grpc::ClientContext *context,
                                   const std::string &client_id,
                                   int deadline_ms) {
        context->AddMetadata("x-token", "demo-token");
        context->AddMetadata("x-client-id", client_id);
        context->set_deadline(std::chrono::system_clock::now() +
                              std::chrono::milliseconds(deadline_ms));
    }

    std::unique_ptr<DemoService::Stub> stub_;
    grpc::CompletionQueue cq_;
    grpc::ClientContext context_;
    grpc::Status status_;
    CountDownRequest request_;
    CountDownReply read_msg_;
    std::unique_ptr<grpc::ClientAsyncReader<CountDownReply>> reader_;
};

// --------------------------------------
// Async bidirectional-stream demo client
// Chat(stream ChatMessage) -> stream ChatMessage
// --------------------------------------
class AsyncBidiChatClient {
public:
    explicit AsyncBidiChatClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    int RunDemo() {
        context_.AddMetadata("x-token", "demo-token");
        context_.AddMetadata("x-client-id", "async-bidi-client");
        context_.set_deadline(std::chrono::system_clock::now() +
                              std::chrono::seconds(10));

        messages_ = {"hello", "how are you", "bye"};

        stream_ = stub_->PrepareAsyncChat(&context_, &cq_);
        stream_->StartCall(Tag(TagType::kStartCall));

        while (true) {
            void *tag = nullptr;
            bool ok = false;
            if (!cq_.Next(&tag, &ok)) {
                std::cerr << "[AsyncBidi] completion queue closed unexpectedly"
                          << std::endl;
                return 1;
            }

            const TagType type = FromTag(tag);

            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] StartCall failed" << std::endl;
                    return 1;
                }
                WriteNext();
                continue;
            }

            if (type == TagType::kWrite) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] Write failed" << std::endl;
                    return 1;
                }
                stream_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            if (type == TagType::kRead) {
                if (!ok) {
                    stream_->Finish(&final_status_, Tag(TagType::kFinish));
                    continue;
                }

                std::cout << "[AsyncBidi] " << read_msg_.user() << ": "
                          << read_msg_.text() << " @" << read_msg_.unix_ms()
                          << std::endl;

                if (send_index_ < messages_.size())
                    WriteNext();
                else
                    stream_->WritesDone(Tag(TagType::kWritesDone));
                continue;
            }

            if (type == TagType::kWritesDone) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] WritesDone failed" << std::endl;
                    return 1;
                }
                stream_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            if (type == TagType::kFinish) {
                if (final_status_.ok()) {
                    std::cout << "[AsyncBidi] finished OK" << std::endl;
                    return 0;
                }
                std::cout << "[AsyncBidi] failed: "
                          << final_status_.error_code() << " "
                          << final_status_.error_message() << std::endl;
                return 1;
            }
        }
    }

private:
    enum class TagType : std::intptr_t {
        kStartCall = 1,
        kWrite = 2,
        kRead = 3,
        kWritesDone = 4,
        kFinish = 5,
    };

    static void *Tag(TagType t) {
        return reinterpret_cast<void *>(static_cast<std::intptr_t>(t));
    }

    static TagType FromTag(void *p) {
        return static_cast<TagType>(reinterpret_cast<std::intptr_t>(p));
    }

    void WriteNext() {
        write_msg_.set_user("async-client");
        write_msg_.set_text(messages_.at(send_index_));
        write_msg_.set_unix_ms(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
        ++send_index_;
        stream_->Write(write_msg_, Tag(TagType::kWrite));
    }

    std::unique_ptr<DemoService::Stub> stub_;
    grpc::CompletionQueue cq_;
    grpc::ClientContext context_;
    grpc::Status final_status_;
    std::unique_ptr<grpc::ClientAsyncReaderWriter<ChatMessage, ChatMessage>>
        stream_;
    std::vector<std::string> messages_;
    std::size_t send_index_ = 0;
    ChatMessage write_msg_;
    ChatMessage read_msg_;
};

} // namespace

int main(int argc, char **argv) {
    const std::string target = argc > 1 ? argv[1] : "127.0.0.1:50052";

    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 20 * 1000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5 * 1000);

    auto channel = grpc::CreateCustomChannel(
        target, grpc::InsecureChannelCredentials(), args);

    AsyncUnaryClient unary_client(channel);
    unary_client.SendHello("alice", 50);
    unary_client.SendHello("bob", 0);
    unary_client.SendHello("charlie", 120);
    unary_client.DrainResponses(3);

    AsyncClientStreamClient client_stream_client(channel);
    if (client_stream_client.RunDemo() != 0) return 1;

    AsyncServerStreamClient server_stream_client(channel);
    if (server_stream_client.RunDemo() != 0) return 1;

    AsyncBidiChatClient bidi_client(channel);
    return bidi_client.RunDemo();
}
