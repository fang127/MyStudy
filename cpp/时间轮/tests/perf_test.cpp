#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "time_wheel.h"

namespace {

/**
 * @brief 压测用 Session 类型。
 */
class Session {
public:
    /**
     * @brief 构造 Session。
     * @param id 会话 ID。
     */
    explicit Session(std::string id) : id_(std::move(id)) {}

    /**
     * @brief 获取会话 ID。
     * @return const std::string& 会话 ID。
     */
    const std::string& Id() const {
        return id_;
    }

private:
    std::string id_;  ///< 会话唯一标识。
};

/**
 * @brief 断言工具，失败时打印并退出。
 * @param cond 断言条件。
 * @param msg 失败信息。
 */
void Require(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

/**
 * @brief 压测入口。
 *
 * 测试阶段：
 * 1) 构造 10000 Session 并注入时间轮。
 * 2) 多线程并发 Refresh，统计吞吐（QPS）。
 * 3) 统一刷新后停止心跳，验证全量超时准确性。
 *
 * @return int 进程退出码。
 */
int main() {
    using clock = std::chrono::steady_clock;

    constexpr int kSessionCount = 10000;
    const unsigned hw = std::thread::hardware_concurrency();
    const int workerCount = static_cast<int>(hw == 0 ? 4 : std::max(4u, hw));

    tw::TimeWheelOptions opt;
    opt.tick = std::chrono::milliseconds(20);
    opt.timeout = std::chrono::milliseconds(200);
    opt.indexShardCount = 128;

    std::mutex expireMutex;
    std::condition_variable expireCv;
    std::atomic<int> expiredCount{0};

    std::vector<std::shared_ptr<Session>> sessions;
    sessions.reserve(kSessionCount);

    std::vector<std::chrono::milliseconds> expireDelays;
    expireDelays.reserve(kSessionCount);

    std::atomic<long long> refreshOps{0};

    auto markStart = clock::now();

    tw::TimeWheel<std::string, Session> wheel(
        opt,
        [](const std::shared_ptr<Session>& s) { return s->Id(); },
        [&](const std::shared_ptr<Session>&) {
            const auto now = clock::now();
            const auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now - markStart);
            {
                std::lock_guard<std::mutex> g(expireMutex);
                expireDelays.push_back(d);
            }
            expiredCount.fetch_add(1, std::memory_order_relaxed);
            expireCv.notify_one();
        });

    wheel.Start();

    // 阶段 1：预热并注入 10000 Session。
    for (int i = 0; i < kSessionCount; ++i) {
        auto s = std::make_shared<Session>("sess-" + std::to_string(i));
        sessions.push_back(s);
        Require(wheel.Refresh(s), "Initial refresh failed");
    }

    std::atomic<bool> running{true};
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    const auto throughputStart = clock::now();

    // 阶段 2：并发刷新压测，模拟高并发心跳。
    for (int w = 0; w < workerCount; ++w) {
        workers.emplace_back([&, w] {
            std::size_t idx = static_cast<std::size_t>(w);
            while (running.load(std::memory_order_acquire)) {
                auto& s = sessions[idx % sessions.size()];
                wheel.Refresh(s);
                refreshOps.fetch_add(1, std::memory_order_relaxed);
                idx += static_cast<std::size_t>(workerCount);
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    running.store(false, std::memory_order_release);
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    const auto throughputEnd = clock::now();
    const double sec = std::chrono::duration<double>(throughputEnd - throughputStart).count();
    const auto ops = refreshOps.load(std::memory_order_relaxed);
    const double qps = sec > 0.0 ? static_cast<double>(ops) / sec : 0.0;

    // 阶段 3：统一刷新后停止心跳，验证全量超时准确性。
    markStart = clock::now();
    for (const auto& s : sessions) {
        wheel.Refresh(s);
    }

    {
        std::unique_lock<std::mutex> lk(expireMutex);
        const auto deadline = clock::now() + std::chrono::seconds(5);
        while (expiredCount.load(std::memory_order_relaxed) < kSessionCount) {
            if (expireCv.wait_until(lk, deadline) == std::cv_status::timeout) {
                break;
            }
        }
    }

    wheel.Stop();

    const int totalExpired = expiredCount.load(std::memory_order_relaxed);
    Require(totalExpired == kSessionCount, "Not all sessions expired in time");
    Require(!expireDelays.empty(), "No expiration delay samples collected");

    auto minDelay = expireDelays.front();
    auto maxDelay = expireDelays.front();
    for (auto d : expireDelays) {
        if (d < minDelay) {
            minDelay = d;
        }
        if (d > maxDelay) {
            maxDelay = d;
        }
    }

    const auto lowerBound = opt.timeout - opt.tick;
    const auto upperBound = opt.timeout + opt.tick * 10;
    Require(minDelay >= lowerBound, "Expiration too early");
    Require(maxDelay <= upperBound, "Expiration too late");

    std::cout << "[PERF] sessions=" << kSessionCount
              << " workers=" << workerCount
              << " refresh_ops=" << ops
              << " qps=" << static_cast<long long>(qps) << "\n";
    std::cout << "[ACCURACY] expired=" << totalExpired
              << " min_delay=" << minDelay.count() << "ms"
              << " max_delay=" << maxDelay.count() << "ms\n";

    return EXIT_SUCCESS;
}
