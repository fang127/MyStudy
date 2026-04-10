#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "time_wheel.h"

class Session
{
public:
    explicit Session(std::string id) : id_(std::move(id)) {}

    const std::string &Id() const { return id_; }

    void MarkZombie() { zombie_.store(true, std::memory_order_relaxed); }

    bool IsZombie() const { return zombie_.load(std::memory_order_relaxed); }

private:
    std::string id_;
    std::atomic<bool> zombie_{
        false}; // 标记连接已过期但尚未清理，供过期回调和外部查询使用。
};

int main()
{
    using namespace std::chrono_literals;

    std::mutex ioMutex;
    std::unordered_map<std::string, std::shared_ptr<Session>> liveSessions;

    tw::TimeWheelOptions options;
    options.tick = std::chrono::seconds(1);
    options.timeout = std::chrono::seconds(90); // 3 * 30s heartbeat

    tw::TimeWheel<std::string, Session> wheel(
        options, [](const std::shared_ptr<Session> &s) { return s->Id(); },
        [&](const std::shared_ptr<Session> &expired)
        {
            // 异步线程内仅做超时标记和引用移除，不阻塞时间轮 tick。
            expired->MarkZombie();
            std::lock_guard<std::mutex> guard(ioMutex);
            liveSessions.erase(expired->Id());
            std::cout << "[EXPIRE] session=" << expired->Id()
                      << " zombie=" << expired->IsZombie() << '\n';
        });

    wheel.start();

    for (int i = 0; i < 5; ++i)
    {
        auto s = std::make_shared<Session>("user-" + std::to_string(i + 1));
        {
            std::lock_guard<std::mutex> guard(ioMutex);
            liveSessions.emplace(s->Id(), s);
        }
        wheel.refresh(s);
    }

    // 模拟网络 IO 线程心跳刷新：前三个连接持续续约，后两个停止心跳并超时。
    std::atomic<bool> running{true};
    std::thread ioThread(
        [&]
        {
            while (running.load(std::memory_order_acquire))
            {
                {
                    std::lock_guard<std::mutex> guard(ioMutex);
                    int refreshed = 0;
                    for (const auto &kv : liveSessions)
                    {
                        if (refreshed >= 3) break;
                        wheel.refresh(kv.second);
                        ++refreshed;
                    }
                    std::cout << "[IO] refreshed=" << refreshed
                              << " active=" << liveSessions.size() << "\n";
                }
                std::this_thread::sleep_for(30s);
            }
        });

    std::this_thread::sleep_for(220s);

    running.store(false, std::memory_order_release);
    if (ioThread.joinable()) ioThread.join();

    wheel.stop();

    std::cout << "[DONE] remain sessions in map=" << liveSessions.size()
              << '\n';
    return EXIT_SUCCESS;
}
