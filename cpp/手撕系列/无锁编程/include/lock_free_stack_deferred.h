#pragma once

#include <atomic>
#include <optional>
#include <utility>

namespace lockfree {

// 延迟回收版 Treiber 栈。
// pop() 完成后不马上 delete，而是先挂到待回收链表。
// 当确认没有其他线程正在执行 pop() 时，再统一删除。
template <typename T>
class LockFreeStackDeferred {
private:
    struct Node {
        T data;
        Node *next;

        explicit Node(T value) : data(std::move(value)), next(nullptr) {}
    };

public:
    ~LockFreeStackDeferred() {
        deleteChain(head_.load(std::memory_order_relaxed));
        deleteChain(retired_.load(std::memory_order_relaxed));
    }

    void push(T value) {
        Node *node = new Node(std::move(value));
        node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(node->next, node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {}
    }

    std::optional<T> pop() {
        activePoppers_.fetch_add(1, std::memory_order_acq_rel);

        Node *oldHead = head_.load(std::memory_order_acquire);
        while (oldHead && !head_.compare_exchange_weak(
                              oldHead, oldHead->next, std::memory_order_acq_rel,
                              std::memory_order_relaxed)) {}

        std::optional<T> result;
        if (oldHead) result = std::move(oldHead->data);

        retire(oldHead);
        return result;
    }

private:
    void retire(Node *node) {
        if (!node) {
            activePoppers_.fetch_sub(1, std::memory_order_acq_rel);
            return;
        }

        pushRetired(node);
        if (activePoppers_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // 说明当前线程是最后一个离开 pop()
            // 的线程，可以安全清理整条待回收链。
            deleteChain(retired_.exchange(nullptr, std::memory_order_acq_rel));
        }
    }

    void pushRetired(Node *node) {
        node->next = retired_.load(std::memory_order_relaxed);
        while (!retired_.compare_exchange_weak(node->next, node,
                                               std::memory_order_release,
                                               std::memory_order_relaxed)) {}
    }

    static void deleteChain(Node *node) {
        while (node) {
            Node *next = node->next;
            delete node;
            node = next;
        }
    }

private:
    std::atomic<Node *> head_{nullptr};
    std::atomic<Node *> retired_{nullptr};
    std::atomic<unsigned> activePoppers_{0};
};

} // namespace lockfree
