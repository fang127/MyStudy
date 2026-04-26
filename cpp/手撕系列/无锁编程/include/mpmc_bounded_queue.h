#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace lockfree {

// 基于 Dmitry Vyukov 思路的有界 MPMC 队列。
// 每个槽位都有一个 sequence，用它判断该槽位当前是“可写”还是“可读”。
template <typename T, std::size_t Capacity>
class MpmcBoundedQueue {
    static_assert(Capacity >= 2, "Capacity must be at least 2");

private:
    struct Cell {
        std::atomic<std::size_t> sequence;
        std::optional<T> storage;
    };

public:
    MpmcBoundedQueue() {
        for (std::size_t i = 0; i < Capacity; ++i)
            cells_[i].sequence.store(i, std::memory_order_relaxed);
    }

    bool enqueue(const T &value) { return emplace(value); }

    bool enqueue(T &&value) { return emplace(std::move(value)); }

    template <typename... Args>
    bool emplace(Args &&...args) {
        std::size_t pos = enqueuePos_.load(std::memory_order_relaxed);
        while (true) {
            Cell &cell = cells_[pos % Capacity];
            const std::size_t seq =
                cell.sequence.load(std::memory_order_acquire);
            const intptr_t diff =
                static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            if (diff == 0) {
                if (enqueuePos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed)) {
                    cell.storage.emplace(std::forward<Args>(args)...);
                    cell.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;
            } else {
                pos = enqueuePos_.load(std::memory_order_relaxed);
            }
        }
    }

    bool dequeue(T &out) {
        std::size_t pos = dequeuePos_.load(std::memory_order_relaxed);
        while (true) {
            Cell &cell = cells_[pos % Capacity];
            const std::size_t seq =
                cell.sequence.load(std::memory_order_acquire);
            const intptr_t diff =
                static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            if (diff == 0) {
                if (dequeuePos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed)) {
                    out = std::move(*cell.storage);
                    cell.storage.reset();
                    cell.sequence.store(pos + Capacity,
                                        std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;
            } else {
                pos = dequeuePos_.load(std::memory_order_relaxed);
            }
        }
    }

private:
    alignas(64) std::array<Cell, Capacity> cells_{};
    alignas(64) std::atomic<std::size_t> enqueuePos_{0};
    alignas(64) std::atomic<std::size_t> dequeuePos_{0};
};

} // namespace lockfree
