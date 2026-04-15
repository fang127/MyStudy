#include <chrono>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
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

void FillCommonMetadata(grpc::ClientContext *context,
                        const std::string &client_id, int deadline_ms) {
    context->AddMetadata("x-token", "demo-token");
    context->AddMetadata("x-client-id", client_id);
    context->set_deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(deadline_ms));
}

/**
 * @brief         客户端流式 RPC 的 Reactor 实现。通过继承
 * grpc::ClientWriteReactor 来实现一个上传数字的客户端流式 RPC。
 * @details       ClientWriteReactot 机制：底层是基于 gRPC 的 CompletionQueue
 * 来实现的。当客户端发起一个客户端流式 RPC 调用时，gRPC 会在后台创建一个
 * CompletionQueue 来管理该 RPC 的生命周期和事件。当客户端调用 StartWrite()
 * 来发送每条消息时，gRPC 会将一个事件放入 CompletionQueue
 * 中，表示消息已发送完成。客户端的 Reactor 通过调用 StartWrite()
 * 来等待这些事件，并在
 * OnWriteDone()回调函数中检查每条消息是否成功发送，并决定是否继续发送下一条消息。当客户端完成发送所有消息时，调用
 * StartWritesDone()
 * 来通知服务器写入完成。最后，当服务器响应结果或发生错误时，gRPC
 * 会将一个完成事件放入 CompletionQueue 中，触发 OnDone() 回调函数来处理 RPC
 * 的结束状态。
 */
class UploadNumbersReactor final : public grpc::ClientWriteReactor<Number> {
public:
    /**
     * @brief         Construct a new Upload Numbers Reactor object
     *
     * @param         stub
     * @param         done
     */
    UploadNumbersReactor(DemoService::Stub *stub, std::promise<int> done)
        : done_(std::move(done)) {
        FillCommonMetadata(&context_, "async-callback-client-stream", 8000);
        numbers_ = {1, 2, 3, 4, 5};

        // 通过 stub->async() 来发起一个异步 RPC 调用，并将当前 Reactor
        // 作为回调对象传入。
        stub->async()->UploadNumbers(&context_, &reply_, this);
        // 调用 StartCall() 来启动 RPC 调用，并在 OnWriteDone() 中通过
        // StartWrite() 来发送每个数字。
        StartCall();
        // 首次调用
        WriteNext();
    }

    /**
     * @brief         当每次 Write 操作完成时，都会调用 OnWriteDone()
     * 回调函数。通过检查
     *
     * @param         ok
     */
    void OnWriteDone(bool ok) override {
        if (!ok) {
            // 如果 Write 失败，直接结束写入并等待服务器响应。
            StartWritesDone();
            return;
        }

        if (index_ < numbers_.size()) {
            WriteNext();
            return;
        }
        // 所有数字都写入完成，调用
        StartWritesDone();
    }

    /**
     * @brief         当 RPC 调用完成时，无论是成功还是失败，都会调用 OnDone()
     * 回调函数。通过检查 status 来判断 RPC
     * 是否成功，并处理服务器返回的结果或错误信息。
     *
     * @param         status
     */
    void OnDone(const grpc::Status &status) override {
        if (status.ok()) {
            std::cout << "[CallbackClientStream] sum=" << reply_.sum()
                      << ", count=" << reply_.count() << std::endl;
            // 如果 RPC 成功，输出服务器返回的 sum 和 count，并将 done_
            // 的值设置为 0 表示成功。 done_ 是一个
            // std::promise<int>，用于通知主线程 RPC 调用的结果。
            done_.set_value(0);
        } else {
            std::cout << "[CallbackClientStream] failed: "
                      << status.error_code() << " " << status.error_message()
                      << std::endl;
            done_.set_value(1);
        }

        delete this;
    }

private:
    /**
     * @brief         WriteNext()
     * 是一个辅助函数，用于发送下一个数字到服务器。它从
     * numbers_向量中获取当前索引的数字，设置到 write_msg_ 中，并调用
     * StartWrite() 来发送消息。每次发送完成后，OnWriteDone()
     * 会被调用来检查是否需要继续发送下一个数字，直到所有数字都发送完成。
     *
     */
    void WriteNext() {
        write_msg_.set_value(numbers_.at(index_));
        ++index_;
        StartWrite(&write_msg_);
    }

    grpc::ClientContext context_;
    SumReply reply_;
    Number write_msg_;
    std::vector<int> numbers_;
    std::size_t index_ = 0;
    std::promise<int> done_;
};

/**
 * @brief         服务器流式 RPC 的 Reactor 实现。通过继承
 * grpc::ClientReadReactor 来实现一个倒计时的服务器流式 RPC。
 * @details       ClientReadReactot 机制：底层是基于 gRPC 的 CompletionQueue
 * 来实现的。当客户端发起一个服务器流式 RPC 调用时，gRPC 会在后台创建一个
 * CompletionQueue 来管理该 RPC 的生命周期和事件。当服务器发送每条消息时，gRPC
 * 会将一个事件放入 CompletionQueue 中，表示有新的消息可供读取。客户端的 Reactor
 * 通过调用 StartRead() 来等待这些事件，并在 OnReadDone()
 * 回调函数中处理每条收到的消息。当服务器完成发送所有消息或发生错误时，gRPC
 * 会将一个完成事件放入 CompletionQueue 中，触发 OnDone() 回调函数来处理 RPC
 * 的结束状态。
 */
class CountDownReactor final : public grpc::ClientReadReactor<CountDownReply> {
public:
    /**
     * @brief         Construct a new Count Down Reactor object
     *
     * @param         stub
     * @param         done
     */
    CountDownReactor(DemoService::Stub *stub, std::promise<int> done)
        : done_(std::move(done)) {
        FillCommonMetadata(&context_, "async-callback-server-stream", 8000);

        request_.set_from(5);
        request_.set_interval_ms(120);

        stub->async()->CountDown(&context_, &request_, this);
        StartCall();
        StartRead(&read_msg_);
    }

    /**
     * @brief         当每次 Read 操作完成时，都会调用 OnReadDone()
     * 回调函数。通过检查 ok
     * 来判断是否成功读取到一条消息，并处理服务器返回的消息内容。如果 ok 为
     * false，表示服务器已经完成发送所有消息或发生错误，客户端应该停止读取并等待
     * RPC 调用的结束状态。
     *
     * @param         ok
     */
    void OnReadDone(bool ok) override {
        if (!ok) return;

        std::cout << "[CallbackServerStream] current=" << read_msg_.current()
                  << std::endl;
        StartRead(&read_msg_);
    }

    /**
     * @brief         当 RPC 调用完成时，无论是成功还是失败，都会调用 OnDone()
     *
     * @param         status
     */
    void OnDone(const grpc::Status &status) override {
        if (status.ok()) {
            std::cout << "[CallbackServerStream] finished OK" << std::endl;
            done_.set_value(0);
        } else {
            std::cout << "[CallbackServerStream] failed: "
                      << status.error_code() << " " << status.error_message()
                      << std::endl;
            done_.set_value(1);
        }

        delete this;
    }

private:
    grpc::ClientContext context_;
    CountDownRequest request_;
    CountDownReply read_msg_;
    std::promise<int> done_;
};

/**
 * @brief         双向流式 RPC 的 Reactor 实现。通过继承 grpc::ClientBidiReactor
 * 来实现一个聊天的双向流式 RPC。
 * @details       ClientBidiReactot 机制：底层是基于 gRPC 的 CompletionQueue
 * 来实现的。当客户端发起一个双向流式 RPC 调用时，gRPC 会在后台创建一个
 * CompletionQueue 来管理该 RPC 的生命周期和事件。当客户端发送每条消息时，gRPC
 * 会将一个事件放入 CompletionQueue
 * 中，表示消息已发送完成。当服务器发送每条消息时，gRPC 也会将一个事件放入
 * CompletionQueue 中，表示有新的消息可供读取。客户端的 Reactor 通过调用
 * StartWrite() 来发送消息，调用 StartRead() 来等待服务器的响应，并在
 * OnWriteDone() 和 OnReadDone()
 * 回调函数中处理每条发送和接收的消息。当客户端完成发送所有消息时，调用
 * StartWritesDone()
 * 来通知服务器写入完成。当服务器完成发送所有消息或发生错误时，gRPC
 * 会将一个完成事件放入 CompletionQueue 中，触发 OnDone() 回调函数来处理 RPC
 * 的结束状态。
 */
class ChatReactor final
    : public grpc::ClientBidiReactor<ChatMessage, ChatMessage> {
public:
    ChatReactor(DemoService::Stub *stub, std::promise<int> done)
        : done_(std::move(done)) {
        FillCommonMetadata(&context_, "async-callback-bidi", 10000);
        messages_ = {"hello", "how are you", "bye"};

        stub->async()->Chat(&context_, this);
        StartCall();
        // 首次调用 StartRead() 来等待服务器的响应，并在 OnReadDone()
        // 中处理每条收到的消息。当服务器完成发送所有消息或发生错误时，gRPC
        // 会将一个完成事件放入 CompletionQueue 中，触发 OnDone() 回调函数来处理
        // RPC 的结束状态。
        // startRead是一个异步操作，表示客户端准备好接收服务器发送的消息。当服务器发送一条消息时，gRPC
        // 会将一个事件放入 CompletionQueue 中，触发 OnReadDone()
        // 回调函数来处理该消息。在 OnReadDone() 中，客户端可以继续调用
        // StartRead() 来等待下一条消息，直到服务器完成发送所有消息或发生错误。
        StartRead(&read_msg_);
        // 首次调用
        WriteNext();
    }

    void OnWriteDone(bool ok) override {
        if (!ok) return;

        if (send_index_ < messages_.size()) {
            WriteNext();
            return;
        }

        StartWritesDone();
    }

    void OnReadDone(bool ok) override {
        if (!ok) return;

        std::cout << "[CallbackBidi] " << read_msg_.user() << ": "
                  << read_msg_.text() << " @" << read_msg_.unix_ms()
                  << std::endl;
        StartRead(&read_msg_);
    }

    void OnDone(const grpc::Status &status) override {
        if (status.ok()) {
            std::cout << "[CallbackBidi] finished OK" << std::endl;
            done_.set_value(0);
        } else {
            std::cout << "[CallbackBidi] failed: " << status.error_code() << " "
                      << status.error_message() << std::endl;
            done_.set_value(1);
        }

        delete this;
    }

private:
    void WriteNext() {
        write_msg_.set_user("callback-client");
        write_msg_.set_text(messages_.at(send_index_));
        write_msg_.set_unix_ms(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
        ++send_index_;
        StartWrite(&write_msg_);
    }

    grpc::ClientContext context_;
    std::vector<std::string> messages_;
    std::size_t send_index_ = 0;
    ChatMessage write_msg_;
    ChatMessage read_msg_;
    std::promise<int> done_;
};

/**
 * @brief         简单 RPC 实现
 *
 * @param         stub
 * @return
 */
int RunUnary(DemoService::Stub *stub) {
    grpc::ClientContext context;
    FillCommonMetadata(&context, "async-callback-unary", 5000);

    HelloRequest request;
    request.set_name("alice");
    request.set_delay_ms(50);

    HelloReply reply;

    std::promise<int> done;
    auto future = done.get_future();

    /**
     * @brief         通过 stub->async() 来发起一个异步 RPC 调用，并将一个
     * lambda 函数作为回调传入。当服务器响应结果或发生错误时，gRPC 会调用该
     * lambda 函数，并将 RPC 的状态作为参数传入。通过检查 status 来判断 RPC
     * 是否成功，并处理服务器返回的结果或错误信息。最后，将 done 的值设置为 0 或
     * 1 来通知主线程 RPC 调用的结果。这里一共有两种回调方式：一种是通过继承
     * grpc::ClientReactor 来实现一个自定义的 Reactor 类，另一种是直接使用
     * lambda 函数作为回调。对于简单的 RPC 调用，使用 lambda
     * 函数可以更简洁地处理结果，而对于复杂的流式 RPC 调用，使用自定义 Reactor
     * 类可以更灵活地管理 RPC 的生命周期和事件。
     * @details       底层其实创建了一个线程来处理这个异步 RPC
     * 调用，并在该线程中等待服务器的响应。当服务器响应结果或发生错误时，gRPC
     * 会在该线程中调用传入的 lambda 函数来处理结果。主线程通过 future.get()
     * 来等待 lambda 函数设置 done 的值，从而获取 RPC 调用的结果。
     *
     */
    stub->async()->SayHello(
        &context, &request, &reply, [&reply, &done](grpc::Status status) {
            if (status.ok()) {
                std::cout << "[CallbackUnary] " << reply.message()
                          << ", server_unix_ms=" << reply.server_unix_ms()
                          << std::endl;
                done.set_value(0);
            } else {
                std::cout << "[CallbackUnary] failed: " << status.error_code()
                          << " " << status.error_message() << std::endl;
                done.set_value(1);
            }
        });

    return future.get();
}

/**
 * @brief         客户端流式 RPC 实现
 *
 * @param         stub
 * @return
 */
int RunClientStream(DemoService::Stub *stub) {
    std::promise<int> done;
    auto future = done.get_future();
    new UploadNumbersReactor(stub, std::move(done));
    return future.get();
}

/**
 * @brief         服务器流式 RPC 实现
 *
 * @param         stub
 * @return
 */
int RunServerStream(DemoService::Stub *stub) {
    std::promise<int> done;
    auto future = done.get_future();
    new CountDownReactor(stub, std::move(done));
    return future.get();
}

/**
 * @brief         双向流式 RPC 实现
 *
 * @param         stub
 * @return
 */
int RunBidi(DemoService::Stub *stub) {
    std::promise<int> done;
    auto future = done.get_future();
    new ChatReactor(stub, std::move(done));
    return future.get();
}

} // namespace

int main(int argc, char **argv) {
    const std::string target = argc > 1 ? argv[1] : "127.0.0.1:50053";

    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 20 * 1000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 5 * 1000);

    auto channel = grpc::CreateCustomChannel(
        target, grpc::InsecureChannelCredentials(), args);

    auto stub = DemoService::NewStub(channel);

    if (RunUnary(stub.get()) != 0) return 1;
    if (RunClientStream(stub.get()) != 0) return 1;
    if (RunServerStream(stub.get()) != 0) return 1;
    return RunBidi(stub.get());
}
