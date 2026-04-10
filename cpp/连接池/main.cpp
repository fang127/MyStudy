#include "connection_pool.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

class MockDbConnection : public IConnection
{
public:
    bool connect() override
    {
        connected_ = randomSuccess(90);
        return connected_;
    }

    void close() noexcept override { connected_ = false; }

    bool ping() override
    {
        if (!connected_) return false;
        if (!randomSuccess(95))
        {
            connected_ = false;
            return false;
        }
        return true;
    }

private:
    static bool randomSuccess(int ratio) { return (std::rand() % 100) < ratio; }

private:
    bool connected_ = false;
};

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    ConnectionPool::Config cfg;
    cfg.minConnections = 2;
    cfg.maxConnections = 8;
    cfg.acquireTimeout = std::chrono::milliseconds(1000);
    cfg.idleTimeout = std::chrono::seconds(30);
    cfg.maintenanceInterval = std::chrono::milliseconds(500);
    cfg.leakTimeout = std::chrono::seconds(3);
    cfg.reconnectAttempts = 3;
    cfg.reconnectBackoff = std::chrono::milliseconds(50);

    auto pool = ConnectionPool::Create(
        cfg, []() { return std::make_unique<MockDbConnection>(); });

    std::atomic<bool> run{true};
    std::vector<std::thread> workers;
    workers.reserve(16);

    for (int i = 0; i < 16; ++i)
    {
        workers.emplace_back(
            [&, i]()
            {
                while (run.load())
                {
                    auto h = pool->acquire(std::chrono::milliseconds(1000));
                    if (!h)
                    {
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(20));
                        continue;
                    }

                    // 模拟连接使用和偶尔的泄漏
                    bool leak = (i == 0) && ((std::rand() % 1000) == 0);
                    if (leak)
                    {
                        auto *leaked =
                            new ConnectionPool::Handle(std::move(*h));
                        (void)leaked;
                    }
                    else
                    {
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(10 + (std::rand() % 20)));
                        h->release();
                    }
                }
            });
    }

    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto s = pool->stats();
        std::cout << "stats total=" << s.totalConnections
                  << " idle=" << s.idleConnections
                  << " in_use=" << s.inUseConnections
                  << " leak_recovered=" << s.leakedRecovered
                  << " reconnect_ok=" << s.reconnectSuccess
                  << " reconnect_fail=" << s.reconnectFail
                  << " create_fail=" << s.createFail << '\n';
    }

    run = false;
    for (auto &t : workers)
        if (t.joinable()) t.join();

    pool->shutdown();
    return 0;
}
