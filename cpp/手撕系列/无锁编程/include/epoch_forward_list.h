#pragma once

#include <atomic>
#include <optional>
#include <utility>
#include <vector>

#include "epoch_reclaimer.h"

namespace lockfree {

// 教学版单向链表：用 CAS 修改头指针，删除节点交给 EBR 回收。
// 这里故意只做 push_front / pop_front / snapshot，聚焦“删除后何时 free”。
template <typename T>
class EpochForwardList {
private:
    struct Node {
        T data;
        Node *next;

        explicit Node(T value) : data(std::move(value)), next(nullptr) {}
    };

public:
    EpochForwardList()
        : participant_(&EpochDomain::instance().registerThread()) {}

    ~EpochForwardList() {
        Node *node = head_.load(std::memory_order_relaxed);
        while (node) {
            Node *next = node->next;
            delete node;
            node = next;
        }
    }

    void pushFront(T value) {
        auto guard = EpochDomain::instance().enter(*participant_);
        (void)guard;

        Node *node = new Node(std::move(value));
        node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(node->next, node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {}
    }

    std::optional<T> popFront() {
        auto guard = EpochDomain::instance().enter(*participant_);
        (void)guard;

        Node *head = head_.load(std::memory_order_acquire);
        while (head && !head_.compare_exchange_weak(
                           head, head->next, std::memory_order_acq_rel,
                           std::memory_order_relaxed)) {}
        if (!head) return std::nullopt;

        T value = std::move(head->data);
        EpochDomain::instance().retire(head);
        return value;
    }

    std::vector<T> snapshot() {
        auto guard = EpochDomain::instance().enter(*participant_);
        (void)guard;

        std::vector<T> values;
        Node *node = head_.load(std::memory_order_acquire);
        while (node) {
            values.push_back(node->data);
            node = node->next;
        }
        return values;
    }

private:
    std::atomic<Node *> head_{nullptr};
    EpochDomain::Participant *participant_;
};

} // namespace lockfree
