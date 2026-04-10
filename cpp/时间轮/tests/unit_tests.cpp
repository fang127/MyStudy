#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "time_wheel.h"

namespace {

/**
 * @brief 单测用 Session 类型。
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
void Expect(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << '\n';
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief 基础边界测试。
 *
 * 验证点：
 * - 空指针刷新。
 * - 重复刷新不重复计数。
 * - 重复移除行为。
 * - 非超时路径不会误触发过期回调。
 */
void TestBasicBoundary() {
    tw::TimeWheelOptions opt;
    opt.tick = std::chrono::milliseconds(20);
    opt.timeout = std::chrono::milliseconds(200);
    opt.indexShardCount = 16;

    std::atomic<int> expiredCount{0};

    tw::TimeWheel<std::string, Session> wheel(
        opt,
        [](const std::shared_ptr<Session>& s) { return s->Id(); },
        [&](const std::shared_ptr<Session>&) { expiredCount.fetch_add(1, std::memory_order_relaxed); });

    wheel.Start();

    Expect(!wheel.Refresh(nullptr), "Refresh(nullptr) should return false");

    auto s = std::make_shared<Session>("s1");
    Expect(wheel.Refresh(s), "First refresh should succeed");
    Expect(wheel.Refresh(s), "Duplicate refresh should succeed");
    Expect(wheel.Size() == 1, "Duplicate refresh should not increase size");

    Expect(wheel.Remove("s1"), "Remove existing session should succeed");
    Expect(!wheel.Remove("s1"), "Remove missing session should fail");

    wheel.Stop();
    Expect(expiredCount.load(std::memory_order_relaxed) == 0, "No expiration expected in boundary test");

    std::cout << "[PASS] TestBasicBoundary\n";
}

/**
 * @brief 超时准确性测试。
 *
 * 验证点：
 * - 会话都能在合理时间窗口内超时。
 * - 过期不会早于 timeout - tick。
 * - 过期不会晚于 timeout + 容忍窗口。
 */
void TestTimeoutAccuracy() {
    using clock = std::chrono::steady_clock;

    tw::TimeWheelOptions opt;
    opt.tick = std::chrono::milliseconds(20);
    opt.timeout = std::chrono::milliseconds(200);
    opt.indexShardCount = 16;

    std::mutex m;
    std::condition_variable cv;
    std::unordered_set<std::string> expiredIds;
    std::vector<std::chrono::milliseconds> delays;

    const auto start = clock::now();

    tw::TimeWheel<std::string, Session> wheel(
        opt,
        [](const std::shared_ptr<Session>& s) { return s->Id(); },
        [&](const std::shared_ptr<Session>& s) {
            const auto now = clock::now();
            const auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            std::lock_guard<std::mutex> g(m);
            expiredIds.insert(s->Id());
            delays.push_back(delay);
            cv.notify_one();
        });

    wheel.Start();

    for (int i = 0; i < 20; ++i) {
        auto s = std::make_shared<Session>("u-" + std::to_string(i));
        Expect(wheel.Refresh(s), "Initial refresh should succeed");
    }

    {
        std::unique_lock<std::mutex> lk(m);
        const auto deadline = clock::now() + std::chrono::seconds(3);
        while (expiredIds.size() < 20) {
            if (cv.wait_until(lk, deadline) == std::cv_status::timeout) {
                break;
            }
        }
    }

    wheel.Stop();

    Expect(expiredIds.size() == 20, "All sessions should expire");
    Expect(!delays.empty(), "Delays should be collected");

    auto minDelay = delays.front();
    auto maxDelay = delays.front();
    for (auto d : delays) {
        if (d < minDelay) {
            minDelay = d;
        }
        if (d > maxDelay) {
            maxDelay = d;
        }
    }

    const auto lowerBound = opt.timeout - opt.tick;
    const auto upperBound = opt.timeout + opt.tick * 6;

    Expect(minDelay >= lowerBound, "Earliest expiration is too early");
    Expect(maxDelay <= upperBound, "Latest expiration is too late");

    std::cout << "[PASS] TestTimeoutAccuracy"
              << " min=" << minDelay.count() << "ms"
              << " max=" << maxDelay.count() << "ms\n";
}

}  // namespace

/**
 * @brief 单元测试入口。
 * @return int 进程退出码。
 */
int main() {
    TestBasicBoundary();
    TestTimeoutAccuracy();
    std::cout << "[PASS] unit_tests all passed\n";
    return EXIT_SUCCESS;
}
