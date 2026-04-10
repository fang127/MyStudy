#include "high_performance_queue.hpp"

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace asio = boost::asio;
using namespace std::chrono_literals;

/**
 * @brief 运行期指标统计。
 */
struct Metrics
{
    /** @brief 成功入队计数。 */
    std::atomic<std::uint64_t> enqueue_ok{0};
    /** @brief 触发背压后的重试计数。 */
    std::atomic<std::uint64_t> enqueue_retry{0};
    /** @brief 消费成功计数。 */
    std::atomic<std::uint64_t> consumed{0};
    /** @brief 单用户乱序计数。 */
    std::atomic<std::uint64_t> order_violation{0};
};

/**
 * @brief 基于 io_context 的异步生产者。
 *
 * 每个生产者绑定一个用户，模拟网络线程把消息投递到共享队列。
 */
class IoMessageProducer : public std::enable_shared_from_this<IoMessageProducer>
{
public:
    /**
     * @brief 构造生产者。
     * @param ioc 所属 io_context。
     * @param queue 共享高性能队列。
     * @param metrics 指标统计对象。
     * @param user_id 用户 ID。
     * @param total_messages 该用户总发包量。
     */
    IoMessageProducer(asio::io_context &ioc, ShardedMessageQueue &queue,
                      Metrics &metrics, std::uint64_t user_id,
                      std::uint64_t total_messages)
        : ioc_(ioc),
          queue_(queue),
          metrics_(metrics),
          timer_(ioc),
          user_id_(user_id),
          total_messages_(total_messages)
    {
    }

    /**
     * @brief 启动生产流程。
     */
    void start() { schedule_once(); }

private:
    /**
     * @brief 调度下一次生产任务。
     */
    void schedule_once()
    {
        auto self = shared_from_this();
        timer_.expires_after(0ms);
        timer_.async_wait(
            [self](const boost::system::error_code &ec)
            {
                if (ec) return;
                self->produce_once();
            });
    }

    /**
     * @brief 生产并尝试入队一条消息。
     *
     * 入队失败时表示触发背压，采用短延时重试。
     */
    void produce_once()
    {
        if (produced_.load(std::memory_order_acquire) >= total_messages_)
            return;

        Message msg;
        msg.user_id = user_id_;
        msg.session_id = user_id_;
        msg.sequence =
            next_sequence_.fetch_add(1, std::memory_order_relaxed) + 1;
        msg.payload = "msg_from_user_" + std::to_string(user_id_) + "_seq_" +
                      std::to_string(msg.sequence);

        if (queue_.try_enqueue(std::move(msg)))
        {
            produced_.fetch_add(1, std::memory_order_relaxed);
            metrics_.enqueue_ok.fetch_add(1, std::memory_order_relaxed);
            schedule_once();
            return;
        }

        metrics_.enqueue_retry.fetch_add(1, std::memory_order_relaxed);
        auto self = shared_from_this();
        timer_.expires_after(1ms);
        timer_.async_wait(
            [self](const boost::system::error_code &ec)
            {
                if (!ec) self->produce_once();
            });
    }

private:
    /** @brief 绑定的 io_context。 */
    asio::io_context &ioc_;
    /** @brief 共享队列引用。 */
    ShardedMessageQueue &queue_;
    /** @brief 指标引用。 */
    Metrics &metrics_;
    /** @brief 异步定时器，用于节拍与退避重试。 */
    asio::steady_timer timer_;
    /** @brief 绑定用户 ID。 */
    std::uint64_t user_id_;
    /** @brief 用户总消息数。 */
    std::uint64_t total_messages_;
    /** @brief 用户消息序列号生成器。 */
    std::atomic<std::uint64_t> next_sequence_{0};
    /** @brief 已成功入队数量。 */
    std::atomic<std::uint64_t> produced_{0};
};

/**
 * @brief 演示 io_context 线程池 + 无锁分片队列 + 多消费者。
 * @return 0 表示运行结束。
 */
int main()
{
    constexpr std::size_t io_threads = 4;
    constexpr std::size_t consumer_threads = 8;
    constexpr std::size_t users =
        128; // 模拟 128 个用户，每个用户一个生产者线程。
    constexpr std::uint64_t messages_per_user = 20000;

    ShardedMessageQueue::QueueConfig cfg;
    cfg.shard_count = consumer_threads;
    cfg.queue_capacity_per_shard = 4096;
    cfg.high_watermark_per_shard = 3200;
    cfg.low_watermark_per_shard = 1600;

    ShardedMessageQueue queue(cfg);
    Metrics metrics;

    // last_seen 用于记录每个用户上次被消费的消息序列号，以检测乱序。
    std::vector<std::atomic<std::uint64_t>> last_seen(users + 1);
    for (auto &v : last_seen) v.store(0, std::memory_order_relaxed);

    std::atomic<bool> stop{false};
    std::vector<std::thread> consumers;
    consumers.reserve(consumer_threads);

    for (std::size_t shard = 0; shard < consumer_threads; ++shard)
    {
        // 每个消费者线程负责一个分片，持续消费直到生产完成并且队列空。
        consumers.emplace_back(
            [&, shard]()
            {
                Message msg;
                while (!stop.load(std::memory_order_acquire) ||
                       queue.approximate_total_depth() > 0)
                {
                    if (queue.try_dequeue(shard, msg))
                    {
                        const std::uint64_t prev = last_seen[msg.user_id].load(
                            std::memory_order_relaxed);
                        if (msg.sequence != prev + 1)
                        {
                            metrics.order_violation.fetch_add(
                                1, std::memory_order_relaxed);
                        }
                        last_seen[msg.user_id].store(msg.sequence,
                                                     std::memory_order_relaxed);
                        metrics.consumed.fetch_add(1,
                                                   std::memory_order_relaxed);
                        continue;
                    }
                    std::this_thread::yield(); // 无消息时让出
                                               // CPU，避免忙等过热。
                }
            });
    }

    std::vector<std::unique_ptr<asio::io_context>> io_contexts;
    io_contexts.reserve(io_threads);
    std::vector<std::shared_ptr<
        asio::executor_work_guard<asio::io_context::executor_type>>>
        works;
    works.reserve(io_threads);

    for (std::size_t i = 0; i < io_threads; ++i)
    {
        io_contexts.push_back(std::make_unique<asio::io_context>());
        // 保持 io_context 运行，直到显式重置 work_guard。
        works.push_back(
            std::make_shared<
                asio::executor_work_guard<asio::io_context::executor_type>>(
                asio::make_work_guard(*io_contexts.back())));
    }

    std::vector<std::thread> ioc_workers;
    ioc_workers.reserve(io_threads);
    for (std::size_t i = 0; i < io_threads; ++i)
        ioc_workers.emplace_back([&, i]() { io_contexts[i]->run(); });

    std::vector<std::shared_ptr<IoMessageProducer>> producers;
    producers.reserve(users);
    for (std::size_t u = 1; u <= users; ++u)
    {
        auto &ioc = *io_contexts[u % io_threads];
        auto p = std::make_shared<IoMessageProducer>(ioc, queue, metrics, u,
                                                     messages_per_user);
        producers.push_back(p);
        asio::post(ioc, [p]() { p->start(); });
    }

    const auto expected_total =
        static_cast<std::uint64_t>(users) * messages_per_user;
    while (metrics.enqueue_ok.load(std::memory_order_acquire) < expected_total)
        std::this_thread::sleep_for(20ms);

    for (auto &w : works) w->reset();
    for (auto &t : ioc_workers) t.join();

    while (metrics.consumed.load(std::memory_order_acquire) < expected_total)
        std::this_thread::sleep_for(20ms);

    stop.store(true, std::memory_order_release);
    for (auto &t : consumers) t.join();

    std::cout << "enqueue_ok=" << metrics.enqueue_ok.load() << '\n';
    std::cout << "enqueue_retry(backpressure)=" << metrics.enqueue_retry.load()
              << '\n';
    std::cout << "consumed=" << metrics.consumed.load() << '\n';
    std::cout << "order_violation=" << metrics.order_violation.load() << '\n';
    std::cout << "final_depth=" << queue.approximate_total_depth() << '\n';

    return 0;
}
