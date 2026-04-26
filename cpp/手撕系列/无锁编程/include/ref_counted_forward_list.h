#pragma once

#include <atomic>
#include <optional>
#include <utility>
#include <vector>

namespace lockfree {

// 教学版“无锁单向链表 + 节点引用计数”。
// 这里只实现 push_front / pop_front / snapshot，重点是展示：
// 1. 头指针通过 CAS 竞争。
// 2. 读取线程先给节点加引用，再访问 next。
// 3. 引用计数归零时再 delete，避免悬空指针。
template <typename T>
class RefCountedForwardList {
private:
    struct Node {
        std::atomic<std::size_t> refs{1};
        T data;
        std::atomic<Node *> next{nullptr};

        explicit Node(T value) : data(std::move(value)) {}
    };

public:
    ~RefCountedForwardList() {
        Node *node = head_.load(std::memory_order_relaxed);
        while (node) {
            Node *next = node->next.load(std::memory_order_relaxed);
            release(node);
            node = next;
        }
    }

    void pushFront(T value) {
        Node *node = new Node(std::move(value));
        node->next.store(head_.load(std::memory_order_relaxed),
                         std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(
            *reinterpret_cast<Node **>(&node->next), node,
            std::memory_order_release, std::memory_order_relaxed)) {}
    }

    std::optional<T> popFront() {
        while (true) {
            Node *head = head_.load(std::memory_order_acquire);
            if (!head) return std::nullopt;

            retain(head);
            if (head != head_.load(std::memory_order_acquire)) {
                release(head);
                continue;
            }

            Node *next = head->next.load(std::memory_order_acquire);
            if (head_.compare_exchange_strong(head, next,
                                              std::memory_order_acq_rel,
                                              std::memory_order_relaxed)) {
                T value = head->data;
                release(head); // 释放 pop() 自己持有的保护引用
                release(head); // 释放链表头原本拥有的那次引用
                return value;
            }
            release(head);
        }
    }

    std::vector<T> snapshot() const {
        std::vector<T> values;
        Node *node = head_.load(std::memory_order_acquire);
        while (node) {
            retain(node);
            values.push_back(node->data);
            Node *next = node->next.load(std::memory_order_acquire);
            release(node);
            node = next;
        }
        return values;
    }

private:
    static void retain(Node *node) {
        node->refs.fetch_add(1, std::memory_order_acquire);
    }

    static void release(Node *node) {
        if (node->refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete node;
    }

private:
    std::atomic<Node *> head_{nullptr};
};

} // namespace lockfree
