#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace lockfree {

// 极简教学版 EBR（Epoch-Based Reclamation）。
// 读写线程进入临界区时登记自己所在纪元；删除节点时记录“退休纪元”；
// 只有当所有活跃线程都已经离开该纪元后，退休节点才能真正释放。
class EpochDomain {
public:
    struct Participant {
        std::atomic<bool> active{false};
        std::atomic<std::size_t> localEpoch{0};
    };

    class Guard {
    public:
        Guard(EpochDomain &domain, Participant &participant)
            : domain_(&domain), participant_(&participant) {
            const std::size_t epoch =
                domain_->globalEpoch_.load(std::memory_order_acquire);
            participant_->localEpoch.store(epoch, std::memory_order_release);
            participant_->active.store(true, std::memory_order_release);
        }

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

        ~Guard() {
            participant_->active.store(false, std::memory_order_release);
        }

    private:
        EpochDomain *domain_;
        Participant *participant_;
    };

    template <std::size_t Slots = 16>
    static EpochDomain &instance() {
        static EpochDomain domain(Slots);
        return domain;
    }

    explicit EpochDomain(std::size_t slots) : participants_(slots) {}

    Participant &registerThread() {
        const auto id = std::this_thread::get_id();
        for (std::size_t i = 0; i < owners_.size(); ++i) {
            std::thread::id empty;
            if (owners_[i].compare_exchange_strong(empty, id,
                                                   std::memory_order_acq_rel)) {
                return participants_[i];
            }
        }
        throw std::runtime_error("no free epoch slots");
    }

    Guard enter(Participant &participant) { return Guard(*this, participant); }

    template <typename T>
    void retire(T *pointer) {
        retired_.push_back({pointer,
                            globalEpoch_.load(std::memory_order_acquire),
                            [](void *p) { delete static_cast<T *>(p); }});
        tryAdvance();
        reclaim();
    }

private:
    struct RetiredNode {
        void *pointer;
        std::size_t retireEpoch;
        std::function<void(void *)> deleter;
    };

    void tryAdvance() {
        const std::size_t current =
            globalEpoch_.load(std::memory_order_acquire);
        for (auto &participant : participants_) {
            if (!participant.active.load(std::memory_order_acquire)) continue;
            if (participant.localEpoch.load(std::memory_order_acquire) !=
                current)
                return;
        }
        globalEpoch_.compare_exchange_strong(current, current + 1,
                                             std::memory_order_acq_rel);
    }

    void reclaim() {
        const std::size_t safeEpoch =
            globalEpoch_.load(std::memory_order_acquire) >= 2
                ? globalEpoch_.load(std::memory_order_acquire) - 2
                : 0;

        auto it = retired_.begin();
        while (it != retired_.end()) {
            if (it->retireEpoch > safeEpoch) {
                ++it;
                continue;
            }
            it->deleter(it->pointer);
            it = retired_.erase(it);
        }
    }

private:
    std::vector<Participant> participants_;
    std::vector<RetiredNode> retired_;
    std::array<std::atomic<std::thread::id>, 16> owners_{};
    std::atomic<std::size_t> globalEpoch_{0};
};

} // namespace lockfree
