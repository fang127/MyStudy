#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace lockfree {

// 固定数量的危险指针槽位，便于教学。
// 生产代码通常会做成按线程动态注册，或结合 TLS 管理生命周期。
class HazardPointerDomain {
public:
    struct Record {
        std::atomic<std::thread::id> owner;
        std::atomic<void *> pointer;
    };

    class Guard {
    public:
        explicit Guard(Record &record) : record_(&record) {}
        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

        Guard(Guard &&other) noexcept : record_(other.record_) {
            other.record_ = nullptr;
        }

        Guard &operator=(Guard &&other) noexcept {
            if (this == &other) return *this;
            reset();
            record_ = other.record_;
            other.record_ = nullptr;
            return *this;
        }

        ~Guard() { reset(); }

        void protect(void *pointer) {
            record_->pointer.store(pointer, std::memory_order_seq_cst);
        }

        void clear() {
            record_->pointer.store(nullptr, std::memory_order_release);
        }

    private:
        void reset() {
            if (!record_) return;
            clear();
            record_->owner.store(std::thread::id{}, std::memory_order_release);
            record_ = nullptr;
        }

    private:
        Record *record_;
    };

    template <std::size_t Slots = 32>
    static HazardPointerDomain &instance() {
        static HazardPointerDomain domain(Slots);
        return domain;
    }

    explicit HazardPointerDomain(std::size_t slots) : records_(slots) {}

    Guard acquireGuard() {
        const auto id = std::this_thread::get_id();
        for (auto &record : records_) {
            std::thread::id empty;
            if (record.owner.compare_exchange_strong(
                    empty, id, std::memory_order_acq_rel)) {
                return Guard(record);
            }
        }
        throw std::runtime_error("no free hazard pointer slots");
    }

    template <typename T>
    void retire(T *pointer) {
        retired_.emplace_back(pointer,
                              [](void *p) { delete static_cast<T *>(p); });
        if (retired_.size() >= scanThreshold_) scan();
    }

    void scan() {
        std::vector<void *> hazards;
        hazards.reserve(records_.size());
        for (auto &record : records_)
            hazards.push_back(record.pointer.load(std::memory_order_acquire));

        auto it = retired_.begin();
        while (it != retired_.end()) {
            if (isHazard(it->pointer, hazards)) {
                ++it;
                continue;
            }
            it->deleter(it->pointer);
            it = retired_.erase(it);
        }
    }

private:
    struct RetiredNode {
        void *pointer;
        std::function<void(void *)> deleter;
    };

    static bool isHazard(void *pointer, const std::vector<void *> &hazards) {
        for (void *hazard : hazards)
            if (hazard == pointer) return true;
        return false;
    }

private:
    std::vector<Record> records_;
    std::vector<RetiredNode> retired_;
    const std::size_t scanThreshold_ = 8;
};

} // namespace lockfree
