#pragma once

#include <atomic>
#include <optional>
#include <utility>

#include "hazard_pointer.h"

namespace lockfree {

// Treiber 栈 + 危险指针。
// 栈顶节点可能刚被别的线程弹出，因此在读取 head 后先发布危险指针，
// 再次确认 head 没变，确认后才能安全解引用。
template <typename T>
class LockFreeStackHazard {
private:
    struct Node {
        T data;
        Node *next;

        explicit Node(T value) : data(std::move(value)), next(nullptr) {}
    };

public:
    ~LockFreeStackHazard() {
        Node *node = head_.load(std::memory_order_relaxed);
        while (node) {
            Node *next = node->next;
            delete node;
            node = next;
        }
        HazardPointerDomain::instance().scan();
    }

    void push(T value) {
        Node *node = new Node(std::move(value));
        node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(node->next, node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {}
    }

    std::optional<T> pop() {
        auto guard = HazardPointerDomain::instance().acquireGuard();

        while (true) {
            Node *oldHead = head_.load(std::memory_order_acquire);
            if (!oldHead) {
                guard.clear();
                return std::nullopt;
            }

            guard.protect(oldHead);
            if (oldHead != head_.load(std::memory_order_acquire)) continue;

            Node *next = oldHead->next;
            if (head_.compare_exchange_strong(oldHead, next,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed)) {
                T value = std::move(oldHead->data);
                guard.clear();
                HazardPointerDomain::instance().retire(oldHead);
                return value;
            }
        }
    }

private:
    std::atomic<Node *> head_{nullptr};
};

} // namespace lockfree
