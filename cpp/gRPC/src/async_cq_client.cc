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

    /**
     * @brief         演示异步unary
     * RPC的调用，客户端发送请求后可以继续做其他事情，稍后再通过completion
     * queue获取响应结果
     *
     * @param         name
     * @param         delay_ms
     */
    void SendHello(const std::string &name, int delay_ms) {
        auto *call = new AsyncUnaryCall;
        FillCommonMetadata(&call->context, "async-unary-client", 5000);

        HelloRequest request;
        request.set_name(name);
        request.set_delay_ms(delay_ms);

        // 通过stub发起异步RPC调用，得到一个responder对象，并在completion
        // queue上注册一个tag（这里直接用call指针），当响应到达时可以通过这个tag获取对应的call对象
        // tag可以是任意的void*指针，通常会指向一个包含RPC状态的结构体，这样在获取响应时就能知道是哪个RPC调用的结果了
        call->responder = stub_->AsyncSayHello(&call->context, request, &cq_);
        // 启动RPC调用，之后可以继续做其他事情，稍后通过completion
        // queue获取响应结果
        call->responder->Finish(&call->reply, &call->status,
                                static_cast<void *>(call));
    }

    /**
     * @brief         从completion
     * queue中获取响应结果，通常会在一个单独的线程中循环调用这个函数
     *
     * @param         expected 预期要获取的响应数量，获取到这么多响应后就返回
     */
    void DrainResponses(int expected) {
        int done = 0;
        while (done < expected) {
            void *tag = nullptr;
            bool ok = false;
            // 从completion
            // queue中获取下一个完成的RPC调用的tag和状态，如果返回false，说明completion
            // queue已经被关闭了，通常是程序要退出了
            if (!cq_.Next(&tag, &ok)) break;

            // 根据tag获取对应的call对象，这里我们直接把call指针作为tag了
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
    /**
     * @brief
     * 填充一些公共的metadata和deadline，演示如何在异步客户端中设置这些参数
     *
     * @param         context
     * @param         client_id
     * @param         deadline_ms
     */
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

    /**
     * @brief         演示异步client-streaming
     * RPC的调用，客户端通过writer对象发送一系列的请求消息，最后调用WritesDone()表示发送完成，然后通过Finish()获取服务器的响应结果
     *
     * @return
     */
    int RunDemo() {
        FillCommonMetadata(&context_, "async-client-stream-client", 8000);

        writer_ = stub_->PrepareAsyncUploadNumbers(&context_, &reply_, &cq_);
        // 启动RPC调用，之后可以继续做其他事情，稍后通过completion
        // queue获取响应结果
        writer_->StartCall(Tag(TagType::kStartCall));

        values_ = {1, 2, 3, 4, 5};

        // 通过一个循环从completion
        // queue中获取RPC调用的状态，直到获取到Finish的结果为止
        while (true) {
            void *tag = nullptr;
            bool ok = false;
            // 通过completion
            // queue获取下一个完成的RPC调用的tag和状态，如果返回false，说明completion
            // queue已经被关闭了，通常是程序要退出了
            if (!cq_.Next(&tag, &ok)) {
                std::cerr << "[AsyncClientStream] completion queue closed "
                             "unexpectedly"
                          << std::endl;
                return 1;
            }

            const TagType type = FromTag(tag);

            // 根据tag的类型来判断是哪个阶段的RPC调用完成了，然后做相应的处理
            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] StartCall failed"
                              << std::endl;
                    return 1;
                }
                // 启动后就可以开始发送请求了，这里我们先发送第一个数字，后续的数字在Write的tag中继续发送
                WriteNext();
                continue;
            }

            // 每当一个Write完成后，如果还有数字要发送，就继续发送下一个数字；如果所有数字都发送完了，就调用WritesDone()表示发送完成了
            if (type == TagType::kWrite) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] Write failed"
                              << std::endl;
                    return 1;
                }

                // Write完成后会再次进入这个循环，获取到这个Write的tag，然后继续发送下一个数字，直到发送完所有数字为止
                if (index_ < values_.size())
                    WriteNext();
                else
                    writer_->WritesDone(Tag(TagType::kWritesDone));
                continue;
            }

            // 当WritesDone完成后，说明所有请求都发送完了，接下来就等待服务器的响应结果了，这时我们调用Finish()来获取最终的状态和响应结果
            if (type == TagType::kWritesDone) {
                if (!ok) {
                    std::cerr << "[AsyncClientStream] WritesDone failed"
                              << std::endl;
                    return 1;
                }
                writer_->Finish(&status_, Tag(TagType::kFinish));
                continue;
            }

            // 最后当获取到Finish的tag时，说明服务器已经处理完了所有请求并返回了结果，我们可以检查状态是否OK，如果OK就打印结果，否则说明RPC调用失败了
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

    /**
     * @brief
     * 发送下一个数字请求，通过writer对象的Write()方法发送一个Number消息，并在completionqueue上注册一个tag，当这个Write操作完成时可以通过这个tag获取到通知，然后继续发送下一个数字，直到发送完所有数字为止
     *
     */
    void WriteNext() {
        write_msg_.set_value(values_.at(index_));
        ++index_;
        writer_->Write(write_msg_, Tag(TagType::kWrite));
    }

    std::unique_ptr<DemoService::Stub>
        stub_; // gRPC生成的stub对象，用于发起RPC调用
    grpc::CompletionQueue
        cq_; // 异步RPC调用的完成队列，所有RPC调用的结果都会通过这个队列来获取
    grpc::ClientContext
        context_;         // RPC调用的上下文，可以设置metadata、deadline等参数
    grpc::Status status_; // RPC调用的最终状态
    SumReply reply_;      // 最后服务器返回的结果
    Number write_msg_;    // 要发送的数字
    std::vector<int> values_; // 要发送的数字列表
    std::size_t index_ = 0;   // 当前要发送的数字的索引
    std::unique_ptr<grpc::ClientAsyncWriter<Number>>
        writer_; // 用于发送client-streaming
                 // RPC的writer对象，通过stub的PrepareAsyncXXX方法创建
};

// --------------------------------------
// Async server-stream demo client
// CountDown(request) -> stream CountDownReply
// --------------------------------------
class AsyncServerStreamClient {
public:
    explicit AsyncServerStreamClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(DemoService::NewStub(std::move(channel))) {}

    /**
     * @brief         演示异步server-streaming
     *
     * @return
     */
    int RunDemo() {
        FillCommonMetadata(&context_, "async-server-stream-client", 8000);

        request_.set_from(5);
        request_.set_interval_ms(120);

        // 通过stub的PrepareAsyncXXX方法创建一个reader对象，这个对象可以用来接收服务器发送的流式响应消息，之后通过这个reader对象的StartCall()方法启动RPC调用，然后就可以通过这个reader对象的Read()方法来接收服务器发送的每条响应消息了
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

            // 根据tag的类型来判断是哪个阶段的RPC调用完成了，然后做相应的处理
            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncServerStream] StartCall failed"
                              << std::endl;
                    return 1;
                }
                reader_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            // 每当一个Read完成后，如果ok为true，说明成功接收到了一条服务器发送的响应消息，我们就打印这个消息，然后继续调用Read()来接收下一条消息；如果ok为false，说明服务器已经发送完了所有响应消息了，这时我们就调用Finish()来获取最终的状态了
            if (type == TagType::kRead) {
                if (!ok) {
                    // finish的作用是告诉gRPC库这个RPC调用已经完成了，之后这个RPC调用的状态和结果就可以通过completion
                    // queue获取到了，这时我们就可以检查status_来看看这个RPC调用是成功了还是失败了
                    reader_->Finish(&status_, Tag(TagType::kFinish));
                    continue;
                }

                std::cout << "[AsyncServerStream] current="
                          << read_msg_.current() << std::endl;
                reader_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            // 当获取到Finish的tag时，说明服务器已经发送完了所有响应消息了，我们可以检查状态是否OK，如果OK就说明这个RPC调用成功了，否则说明RPC调用失败了
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
        // 通过stub的PrepareAsyncXXX方法创建一个stream对象，这个对象既可以用来发送请求消息，也可以用来接收响应消息，之后通过这个stream对象的StartCall()方法启动RPC调用，然后就可以通过这个stream对象的Read()和Write()方法来发送和接收消息了
        stream_ = stub_->PrepareAsyncChat(&context_, &cq_);
        stream_->StartCall(Tag(TagType::kStartCall));

        while (true) {
            void *tag = nullptr;
            bool ok = false;
            // 通过completionqueue获取下一个完成的RPC调用的tag和状态，如果返回false，说明completionqueue已经被关闭了，通常是程序要退出了
            if (!cq_.Next(&tag, &ok)) {
                std::cerr << "[AsyncBidi] completion queue closed unexpectedly"
                          << std::endl;
                return 1;
            }

            // 根据tag的类型来判断是哪个阶段的RPC调用完成了，然后做相应的处理
            const TagType type = FromTag(tag);

            // 启动后就可以开始发送请求了，这里我们先发送第一个消息，后续的消息在Write的tag中继续发送
            if (type == TagType::kStartCall) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] StartCall failed" << std::endl;
                    return 1;
                }
                WriteNext();
                continue;
            }

            // 每当一个Write完成后，如果还有消息要发送，就继续发送下一条消息；如果所有消息都发送完了，就调用WritesDone()表示发送完成了
            if (type == TagType::kWrite) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] Write failed" << std::endl;
                    return 1;
                }
                // Write完成后会再次进入这个循环，获取到这个Write的tag，然后继续发送下一条消息，直到发送完所有消息为止
                stream_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            // 每当一个Read完成后，如果ok为true，说明成功接收到了一条服务器发送的响应消息，我们就打印这个消息，然后继续调用Read()来接收下一条消息；如果ok为false，说明服务器已经发送完了所有响应消息了，这时我们就调用Finish()来获取最终的状态了
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

            // 当获取到WritesDone的tag时，说明所有请求都发送完了，接下来就等待服务器的响应结果了，这时我们就调用Finish()来获取最终的状态了
            if (type == TagType::kWritesDone) {
                if (!ok) {
                    std::cerr << "[AsyncBidi] WritesDone failed" << std::endl;
                    return 1;
                }
                stream_->Read(&read_msg_, Tag(TagType::kRead));
                continue;
            }

            // 最后当获取到Finish的tag时，说明服务器已经处理完了所有请求并返回了结果，我们可以检查状态是否OK，如果OK就打印结果，否则说明RPC调用失败了
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

    // 演示unary RPC
    AsyncUnaryClient unary_client(channel);
    unary_client.SendHello("alice", 50);
    unary_client.SendHello("bob", 0);
    unary_client.SendHello("charlie", 120);
    // 丢弃掉第3个请求的响应，演示deadline超时的情况
    unary_client.DrainResponses(3);

    // 演示client-streaming RPC
    AsyncClientStreamClient client_stream_client(channel);
    if (client_stream_client.RunDemo() != 0) return 1;

    // 演示server-streaming RPC
    AsyncServerStreamClient server_stream_client(channel);
    if (server_stream_client.RunDemo() != 0) return 1;

    // 演示bidirectional-streaming RPC
    AsyncBidiChatClient bidi_client(channel);
    return bidi_client.RunDemo();
}
