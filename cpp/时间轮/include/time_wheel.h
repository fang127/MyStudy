#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tw
{

/**
 * @brief 为 std::shared_ptr 提供基于裸指针地址的哈希。
 *
 * @tparam T shared_ptr 持有的对象类型。
 */
template <typename T>
struct SharedPtrHash
{
    /**
     * @brief 计算 shared_ptr 的哈希值。
     * @param p 目标 shared_ptr。
     * @return std::size_t 哈希值。
     */
    std::size_t operator()(const std::shared_ptr<T> &p) const noexcept
    {
        return std::hash<T *>{}(p.get());
    }
};

/**
 * @brief 为 std::shared_ptr 提供基于裸指针地址的相等比较。
 *
 * @tparam T shared_ptr 持有的对象类型。
 */
template <typename T>
struct SharedPtrEqual
{
    /**
     * @brief 判断两个 shared_ptr 是否指向同一对象。
     * @param a 左值。
     * @param b 右值。
     * @return true 指向同一对象。
     * @return false 指向不同对象。
     */
    bool operator()(const std::shared_ptr<T> &a,
                    const std::shared_ptr<T> &b) const noexcept
    {
        return a.get() == b.get();
    }
};

/**
 * @brief 时间轮配置项。
 */
struct TimeWheelOptions
{
    std::chrono::milliseconds tick{1000};     // 槽位推进精度，默认 1 秒。
    std::chrono::milliseconds timeout{90000}; // 会话超时时间，默认 90 秒。
    std::size_t indexShardCount{64};          // 索引分片数量，越大锁竞争越低。
};

/**
 * @brief 线程安全高性能时间轮，适用于 IM Session 超时管理。
 *
 * 设计要点：
 * - 槽位环形数组，空间固定。
 * - 索引按 Key 分片，降低全局锁竞争。
 * - 刷新路径为 O(1) 均摊：删旧槽 + 加新槽。
 * - Tick 线程只做过期判定，业务回调异步执行。
 *
 * @tparam Key Session 唯一键类型（需可哈希）。
 * @tparam Value Session 对象类型。
 */
template <typename Key, typename Value>
class TimeWheel
{
public:
    using SessionPtr = std::shared_ptr<Value>; // Session 智能指针类型。
    using KeyExtractor = std::function<Key(
        const SessionPtr &)>; // 从 Session 提取 Key 的函数类型。
    using ExpireHandler =
        std::function<void(const SessionPtr &)>; // 过期异步回调函数类型。

    /**
     * @brief 构造时间轮。
     * @param options 时间轮配置。
     * @param extractor Session->Key 提取器。
     * @param onExpire 会话过期处理回调。
     * @throw std::invalid_argument 配置或回调非法时抛出。
     */
    TimeWheel(TimeWheelOptions options, KeyExtractor extractor,
              ExpireHandler onExpire)
        : options_(options),
          keyExtractor_(std::move(extractor)),
          onExpire_(std::move(onExpire))
    {
        if (!keyExtractor_)
            throw std::invalid_argument("key extractor cannot be empty");
        if (!onExpire_)
            throw std::invalid_argument("expire handler cannot be empty");
        if (options_.tick.count() <= 0 || options_.timeout.count() <= 0)
            throw std::invalid_argument("tick/timeout must be positive");
        if (options_.timeout < options_.tick)
            throw std::invalid_argument("timeout must be >= tick");
        if (options_.indexShardCount == 0)
            throw std::invalid_argument("indexShardCount must be positive");

        // 计算超时对应的槽位跨度，至少为 1。
        timeoutTicks_ =
            static_cast<std::size_t>(options_.timeout / options_.tick);
        if (timeoutTicks_ == 0) timeoutTicks_ = 1;

        // 初始化环形槽位与对应互斥锁。
        slots_.resize(timeoutTicks_);
        slotMutexes_.reserve(timeoutTicks_);
        for (std::size_t i = 0; i < timeoutTicks_; ++i)
            slotMutexes_.emplace_back(std::make_unique<std::mutex>());

        // 初始化索引分片，每个分片单独加锁。
        indexShards_.reserve(options_.indexShardCount);
        for (std::size_t i = 0; i < options_.indexShardCount; ++i)
            indexShards_.emplace_back(std::make_unique<IndexShard>());
    }

    /**
     * @brief 析构函数，自动停止后台线程并释放内部引用。
     */
    ~TimeWheel() { stop(); }

    TimeWheel(const TimeWheel &) = delete;
    TimeWheel &operator=(const TimeWheel &) = delete;

    /**
     * @brief 启动 Tick 线程和过期处理线程。
     *
     * 重复调用是安全的，只有第一次生效。
     */
    void start()
    {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true)) return;

        tickThread_ = std::thread([this] { tickLoop(); });
        expireThread_ = std::thread([this] { expireLoop(); });
    }

    /**
     * @brief 停止时间轮并清理内部状态。
     *
     * 停止后：
     * - 等待两个后台线程退出。
     * - 清空索引、槽位与过期队列。
     * - 移除所有内部持有引用，方便 Session 自动释放。
     */
    void stop()
    {
        bool expected = true;
        // 先将运行状态置为 false，通知线程退出循环。重复调用时只有第一次有效。
        if (!running_.compare_exchange_strong(expected, false)) return;

        queueCv_.notify_all();

        if (tickThread_.joinable()) tickThread_.join();
        if (expireThread_.joinable()) expireThread_.join();

        // 清空哈希索引分片，释放对 Session 的弱引用。
        for (auto &shard : indexShards_)
        {
            std::lock_guard<std::mutex> shardLock(shard->mutex);
            shard->index.clear();
        }
        // 清空槽位，释放对 Session 的强引用。
        for (std::size_t i = 0; i < slots_.size(); ++i)
        {
            std::lock_guard<std::mutex> slotLock(*slotMutexes_[i]);
            slots_[i].clear();
        }

        // 清空过期队列，释放对 Session 的强引用。
        {
            std::lock_guard<std::mutex> queueLock(queueMutex_);
            std::queue<SessionPtr> empty;
            std::swap(expireQueue_, empty);
        }
    }

    /**
     * @brief 刷新会话活跃时间，重置其超时位置。
     *
     * 算法流程（均摊 O(1)）：
     * 1) 按 Key 选分片并加锁。
     * 2) 若已存在，执行“删旧槽 + 加新槽”。
     * 3) 若不存在，直接插入新槽与索引。
     *
     * @param session 要刷新的会话。
     * @return true 成功刷新或插入。
     * @return false 入参为空。
     */
    bool refresh(const SessionPtr &session)
    {
        if (!session) return false;

        Key key = keyExtractor_(session); // 提取 Key 以确定索引位置
        const std::size_t newSlot = computeFutureSlot(); // 计算新槽位索引

        auto *shard = selectShard(key); // 选择索引分片
        std::lock_guard<std::mutex> shardLock(shard->mutex);

        auto iter = shard->index.find(key);
        if (iter != shard->index.end())
        {
            auto oldSession = iter->second.session.lock();
            const std::size_t oldSlot = iter->second.slotIndex;

            if (oldSession)
            {
                // 如果新旧槽位相同，只需更新会话对象，无需迁移。
                if (oldSlot == newSlot)
                {
                    std::lock_guard<std::mutex> slotLock(
                        *slotMutexes_[newSlot]);
                    slots_[newSlot].erase(oldSession);
                    slots_[newSlot].insert(session);
                }
                else
                {
                    // 同时锁定两个槽，保证迁移原子性。
                    std::scoped_lock slotLock(*slotMutexes_[oldSlot],
                                              *slotMutexes_[newSlot]);
                    slots_[oldSlot].erase(oldSession);
                    slots_[newSlot].insert(session);
                }
            }
            else
            {
                // 索引条目存在但 weak_ptr 已失效，视为重建。
                std::lock_guard<std::mutex> slotLock(*slotMutexes_[newSlot]);
                slots_[newSlot].insert(session);
            }

            iter->second.slotIndex = newSlot;
            iter->second.session = session;
            return true;
        }

        {
            std::lock_guard<std::mutex> slotLock(*slotMutexes_[newSlot]);
            slots_[newSlot].insert(session);
        }

        shard->index.emplace(std::move(key), IndexEntry{newSlot, session});
        return true;
    }

    /**
     * @brief 按 Key 主动移除会话。
     * @param key 会话唯一键。
     * @return true 找到并移除成功。
     * @return false 未找到对应会话。
     */
    bool remove(const Key &key)
    {
        auto *shard = selectShard(key);
        std::lock_guard<std::mutex> shardLock(shard->mutex);

        auto iter = shard->index.find(key);
        if (iter == shard->index.end()) return false;

        auto session = iter->second.session.lock();
        const std::size_t slot = iter->second.slotIndex;

        if (session)
        {
            std::lock_guard<std::mutex> slotLock(*slotMutexes_[slot]);
            slots_[slot].erase(session);
        }

        shard->index.erase(iter);
        return true;
    }

    /**
     * @brief 获取当前索引中的会话数量。
     * @return std::size_t 会话总数。
     */
    std::size_t Size() const
    {
        std::size_t total = 0;
        for (const auto &shard : indexShards_)
        {
            std::lock_guard<std::mutex> shardLock(shard->mutex);
            total += shard->index.size();
        }
        return total;
    }

private:
    using Slot = std::unordered_set<SessionPtr, SharedPtrHash<Value>,
                                    SharedPtrEqual<Value>>; // 单个槽位容器。

    /**
     * @brief 索引项：记录会话当前所在槽位及弱引用。
     */
    struct IndexEntry
    {
        std::size_t slotIndex;        // 会话当前所在槽位。
        std::weak_ptr<Value> session; // 弱引用，防止索引强持有导致泄漏。
    };

    /**
     * @brief 索引分片：每片一个 map + 一把锁。
     */
    struct IndexShard
    {
        mutable std::mutex mutex;                  // 分片锁。
        std::unordered_map<Key, IndexEntry> index; // 分片内索引表。
    };

    /**
     * @brief 根据 Key 选择索引分片（可写）。
     * @param key 会话键。
     * @return IndexShard* 分片指针。
     */
    IndexShard *selectShard(const Key &key)
    {
        const std::size_t idx = keyHasher_(key) % indexShards_.size();
        return indexShards_[idx].get();
    }

    /**
     * @brief 根据 Key 选择索引分片（只读）。
     * @param key 会话键。
     * @return const IndexShard* 分片指针。
     */
    const IndexShard *selectShard(const Key &key) const
    {
        const std::size_t idx = keyHasher_(key) % indexShards_.size();
        return indexShards_[idx].get();
    }

    /**
     * @brief 计算“超时应落入的未来槽位”。
     * @return std::size_t 目标槽位索引。
     */
    std::size_t computeFutureSlot() const
    {
        const std::size_t current =
            currentIndex_.load(std::memory_order_relaxed); // 当前槽位索引
        return (current + timeoutTicks_) % timeoutTicks_;
    }

    /**
     * @brief Tick 主循环：推进时间轮并筛选过期会话。
     *
     * 关键行为：
     * - 先锁槽位批量摘取候选，再释放槽位锁。
     * - 逐个会话到分片索引做“二次确认”。
     * - 只把真正超时的会话放入异步过期队列。
     */
    void tickLoop()
    {
        while (running_.load(std::memory_order_acquire))
        {
            std::this_thread::sleep_for(options_.tick);

            // 再次检查运行状态，避免在 sleep 期间被 stop() 请求停止后继续执行。
            if (!running_.load(std::memory_order_acquire)) break;

            const std::size_t next =
                (currentIndex_.load(std::memory_order_relaxed) + 1) %
                timeoutTicks_;
            currentIndex_.store(next, std::memory_order_relaxed);

            // 批量摘取候选会话，减少锁持有时间。即使有新会话进入该槽位，也不会影响当前批次的过期判定。
            std::vector<SessionPtr> candidates;
            {
                std::lock_guard<std::mutex> slotLock(*slotMutexes_[next]);
                if (!slots_[next].empty())
                {
                    candidates.reserve(slots_[next].size());
                    for (const auto &s : slots_[next]) candidates.push_back(s);
                    slots_[next].clear();
                }
            }

            if (candidates.empty()) continue;

            for (const auto &session : candidates)
            {
                if (!session) continue;

                const Key key = keyExtractor_(session);
                auto *shard = SelectShard(key);
                bool expired = false;

                {
                    std::lock_guard<std::mutex> shardLock(shard->mutex);
                    auto iter = shard->index.find(key);
                    if (iter != shard->index.end())
                    {
                        auto latest = iter->second.session.lock();
                        // 二次确认：索引仍指向当前槽且对象身份一致，才认定真正超时。
                        if (iter->second.slotIndex == next && latest &&
                            latest.get() == session.get())
                        {
                            shard->index.erase(iter);
                            expired = true;
                        }
                    }
                }

                if (expired) EnqueueExpired(session);
            }
        }
    }

    /**
     * @brief 将过期会话推入异步处理队列。
     * @param session 已确认过期的会话。
     */
    void EnqueueExpired(const SessionPtr &session)
    {
        {
            std::lock_guard<std::mutex> queueLock(queueMutex_);
            expireQueue_.push(session);
        }
        queueCv_.notify_one();
    }

    /**
     * @brief 过期处理线程循环。
     *
     * 该线程阻塞等待队列，不参与时间轮推进。业务层可在回调中执行
     * 标记僵尸、异步关闭连接等操作，避免阻塞 Tick 线程。
     */
    void expireLoop()
    {
        while (true)
        {
            SessionPtr session;

            {
                std::unique_lock<std::mutex> queueLock(queueMutex_);
                // 等待条件：队列非空或时间轮停止。即使时间轮停止了，也要处理完当前队列中的过期会话。
                queueCv_.wait(
                    queueLock,
                    [this]
                    {
                        return !expireQueue_.empty() ||
                               !running_.load(std::memory_order_acquire);
                    });

                if (expireQueue_.empty() &&
                    !running_.load(std::memory_order_acquire))
                {
                    break;
                }

                session = expireQueue_.front();
                expireQueue_.pop();
            }

            if (session) onExpire_(session);
        }
    }

private:
    const TimeWheelOptions options_;  // 时间轮配置。
    const KeyExtractor keyExtractor_; // Session -> Key 提取器。
    const ExpireHandler onExpire_;    // 过期回调（异步线程执行）。

    std::size_t timeoutTicks_{1};              // 超时对应的槽跨度。
    std::atomic<std::size_t> currentIndex_{0}; // 当前时间轮指针。

    std::vector<Slot> slots_;                              // 环形槽位数组。
    std::vector<std::unique_ptr<std::mutex>> slotMutexes_; // 每个槽位独立锁。

    std::hash<Key> keyHasher_;                             // Key 哈希器。
    std::vector<std::unique_ptr<IndexShard>> indexShards_; // 分片索引集合。

    std::mutex queueMutex_;              // 过期队列锁。
    std::condition_variable queueCv_;    // 过期队列条件变量。
    std::queue<SessionPtr> expireQueue_; // 已确认过期的待处理队列。

    std::atomic<bool> running_{false}; // 运行状态标记。
    std::thread tickThread_;           // Tick 推进线程。
    std::thread expireThread_;         // 过期回调线程。
};

} // namespace tw
