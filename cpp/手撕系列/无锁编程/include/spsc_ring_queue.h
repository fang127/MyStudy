#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace lockfree {

// 教学版 SPSC 环形队列。
// 只有一个生产者线程写 tail_，只有一个消费者线程写 head_。
// 因为槽位预先分配，所以不存在节点回收问题，这是无 GC 语言里最简单安全的设计。
template <typename T, std::size_t Capacity>
class SpscRingQueue {
    static_assert(Capacity >= 2, "Capacity must be at least 2");

public:
    bool push(const T &value) { return emplace(value); }

    bool push(T &&value) { return emplace(std::move(value)); }

    template <typename... Args>
    bool emplace(Args &&...args) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t next = increment(tail);

        // 读 head_ 要用 acquire，确保看到消费者释放出来的槽位。
        if (next == head_.load(std::memory_order_acquire)) return false;

        slots_[tail].emplace(std::forward<Args>(args)...);
        tail_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T &out) {
        const std::size_t head = head_.load(std::memory_order_relaxed);

        // 读 tail_ 要用 acquire，确保看到生产者已经构造完成的对象。
        if (head == tail_.load(std::memory_order_acquire)) return false;

        out = std::move(*slots_[head]);
        slots_[head].reset();
        head_.store(increment(head), std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

private:
    static constexpr std::size_t increment(std::size_t index) {
        return (index + 1) % Capacity;
    }

private:
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    std::array<std::optional<T>, Capacity> slots_{};
};

} // namespace lockfree
