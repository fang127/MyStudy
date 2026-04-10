#pragma once

#include <chrono>
#include <cstdint>
#include <string>

/**
 * @brief 单条业务消息模型。
 *
 * 该结构体由网络线程读取 socket 后构造，并投递到高性能队列中。
 */
struct Message
{
    /** @brief 用户 ID，用于分片和单用户有序性路由。 */
    std::uint64_t user_id{0};
    /** @brief 会话 ID，可用于会话级处理。 */
    std::uint64_t session_id{0};
    /** @brief 用户内递增序号，用于有序性校验。 */
    std::uint64_t sequence{0};
    /** @brief 消息内容载荷。 */
    std::string payload;
    /** @brief 入队时间戳，可用于延迟统计。 */
    std::chrono::steady_clock::time_point enqueued_at;
};
