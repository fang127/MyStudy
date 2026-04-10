#pragma once

#include "message.hpp"

#include <folly/MPMCQueue.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * @brief 基于 folly::MPMCQueue 的分片无锁消息队列。
 *
 * 设计要点：
 * - 有界队列：每个分片固定容量。
 * - 无锁：依赖 MPMCQueue 与原子变量，不使用 mutex。
 * - 背压：按分片高低水位开关 pause 标记。
 * - 单用户有序：按 user_id 固定映射到同一分片。
 */
class ShardedMessageQueue
{
public:
    /**
     * @brief 队列配置。
     */
    struct QueueConfig
    {
        /** @brief 分片数量，通常与消费者线程数一致。 */
        std::size_t shard_count{4};
        /** @brief 每个分片的有界容量。 */
        std::size_t queue_capacity_per_shard{4096};
        /** @brief 每个分片高水位，达到后触发背压。 */
        std::size_t high_watermark_per_shard{3276};
        /** @brief 每个分片低水位，回落后解除背压。 */
        std::size_t low_watermark_per_shard{1638};
    };

    /**
     * @brief 构造分片队列。
     * @param config 队列配置，内部会做合法化修正。
     */
    explicit ShardedMessageQueue(QueueConfig config)
        : config_(sanitize_config(config))
    {
        shards_.reserve(config_.shard_count);
        for (std::size_t i = 0; i < config_.shard_count; ++i)
        {
            shards_.push_back(
                std::make_unique<Shard>(config_.queue_capacity_per_shard));
        }
    }

    /**
     * @brief 尝试入队一条消息。
     * @param msg 待入队消息。
     * @return true 入队成功；false 表示当前背压或队列已满。
     */
    bool try_enqueue(Message msg)
    {
        Shard &shard = shard_for_user(msg.user_id);

        // 先检查背压状态，避免不必要的入队尝试。
        if (shard.pause.load(std::memory_order_acquire)) return false;

        msg.enqueued_at = std::chrono::steady_clock::now();

        if (!shard.queue.write(std::move(msg)))
        {
            shard.pause.store(true, std::memory_order_release);
            return false;
        }

        // 成功入队后更新深度计数，并根据高水位调整背压状态。
        const std::size_t depth =
            shard.depth.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (depth >= config_.high_watermark_per_shard)
            shard.pause.store(true, std::memory_order_release);
        return true;
    }

    /**
     * @brief 从指定消费者分片尝试出队。
     * @param shard_index 消费者分片索引。
     * @param out 出队消息输出参数。
     * @return true 成功取到消息；false 当前无可消费消息。
     */
    bool try_dequeue(std::size_t shard_index, Message &out)
    {
        Shard &shard = *shards_[shard_index % config_.shard_count];
        if (!shard.queue.read(out)) return false;

        const std::size_t depth =
            shard.depth.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (depth <= config_.low_watermark_per_shard)
            shard.pause.store(false, std::memory_order_release);
        return true;
    }

    /**
     * @brief 查询某用户所属分片是否处于背压状态。
     * @param user_id 用户 ID。
     * @return true 需要暂停该用户继续投递。
     */
    bool should_pause(std::uint64_t user_id) const
    {
        const Shard &shard = *shards_[shard_index_for_user(user_id)];
        return shard.pause.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取分片总数。
     * @return 分片数量。
     */
    std::size_t shard_count() const { return config_.shard_count; }

    /**
     * @brief 估算全队列深度。
     * @return 所有分片深度之和（近似值）。
     */
    std::size_t approximate_total_depth() const
    {
        std::size_t total = 0;
        for (const auto &shard : shards_)
            total += shard->depth.load(std::memory_order_acquire);
        return total;
    }

private:
    /**
     * @brief 单分片状态。
     */
    struct Shard
    {
        /**
         * @brief 构造单分片。
         * @param cap 分片容量。
         */
        explicit Shard(std::size_t cap) : queue(cap) {}

        /** @brief folly 无锁有界 MPMC 队列。 */
        folly::MPMCQueue<Message> queue;
        /** @brief 分片深度计数。 */
        std::atomic<std::size_t> depth{0};
        /** @brief 背压开关。 */
        std::atomic<bool> pause{false};
    };

    /**
     * @brief 合法化配置参数。
     * @param cfg 原始配置。
     * @return 修正后的可用配置。
     */
    static QueueConfig sanitize_config(QueueConfig cfg)
    {
        if (cfg.shard_count == 0) cfg.shard_count = 1;
        if (cfg.queue_capacity_per_shard < 8) cfg.queue_capacity_per_shard = 8;
        if (cfg.high_watermark_per_shard == 0 ||
            cfg.high_watermark_per_shard > cfg.queue_capacity_per_shard)
        {
            cfg.high_watermark_per_shard =
                cfg.queue_capacity_per_shard * 8 / 10;
        }
        if (cfg.low_watermark_per_shard >= cfg.high_watermark_per_shard)
            cfg.low_watermark_per_shard = cfg.high_watermark_per_shard / 2;
        return cfg;
    }

    /**
     * @brief 根据用户 ID 计算其分片索引。
     * @param user_id 用户 ID。
     * @return 分片索引。
     */
    std::size_t shard_index_for_user(std::uint64_t user_id) const
    {
        return static_cast<std::size_t>(user_id % config_.shard_count);
    }

    /**
     * @brief 获取用户对应分片。
     * @param user_id 用户 ID。
     * @return 用户所在分片引用。
     */
    Shard &shard_for_user(std::uint64_t user_id)
    {
        return *shards_[shard_index_for_user(user_id)];
    }

private:
    /** @brief 队列配置。 */
    QueueConfig config_;
    /** @brief 分片数组。 */
    std::vector<std::unique_ptr<Shard>> shards_;
};
