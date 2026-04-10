#include <atomic>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

template <typename T>
class LockFreeQueue
{
private:
    struct Node
    {
        std::optional<T> data; // 哨兵节点 data 为空
        std::atomic<Node *> next{nullptr};
        Node *reclaim_next = nullptr; // 仅用于延迟回收链
        Node() = default;
        explicit Node(T v) : data(std::move(v)) {}
    };

public:
    LockFreeQueue()
    {
        Node *dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
        retired_.store(nullptr);
    }

    ~LockFreeQueue()
    {
        // 1) 回收当前队列链上的节点（要求析构时无并发访问）
        Node *cur = head_.load();
        while (cur)
        {
            Node *nxt = cur->next.load();
            delete cur;
            cur = nxt;
        }

        // 2) 回收运行期退役节点
        Node *r = retired_.load();
        while (r)
        {
            Node *nxt = r->reclaim_next;
            delete r;
            r = nxt;
        }
    }

    void push(const T &value)
    {
        Node *node = new Node(value);

        while (true)
        {
            Node *tail = tail_.load(std::memory_order_acquire);
            Node *next = tail->next.load(std::memory_order_acquire);

            if (tail == tail_.load(std::memory_order_acquire))
            {
                if (next == nullptr)
                {
                    // 尝试把新节点挂到 tail->next
                    if (tail->next.compare_exchange_weak(
                            next, node, std::memory_order_release,
                            std::memory_order_relaxed))
                    {
                        // 尝试推进 tail（失败也没关系，别的线程会帮忙）
                        tail_.compare_exchange_weak(tail, node,
                                                    std::memory_order_release,
                                                    std::memory_order_relaxed);
                        return;
                    }
                }
                else
                {
                    // tail 落后，帮忙推进
                    tail_.compare_exchange_weak(tail, next,
                                                std::memory_order_release,
                                                std::memory_order_relaxed);
                }
            }
        }
    }

    bool pop(T &out)
    {
        while (true)
        {
            Node *head = head_.load(std::memory_order_acquire);
            Node *tail = tail_.load(std::memory_order_acquire);
            Node *next = head->next.load(std::memory_order_acquire);

            if (head == head_.load(std::memory_order_acquire))
            {
                if (next == nullptr) return false; // 空队列

                if (head == tail)
                {
                    // tail 落后，帮忙推进
                    tail_.compare_exchange_weak(tail, next,
                                                std::memory_order_release,
                                                std::memory_order_relaxed);
                    continue;
                }

                // 先读数据，再尝试移动 head
                out = *(next->data);
                if (head_.compare_exchange_weak(head, next,
                                                std::memory_order_release,
                                                std::memory_order_relaxed))
                {
                    retire_node(head); // 旧哨兵延迟回收
                    return true;
                }
            }
        }
    }

private:
    void retire_node(Node *node)
    {
        Node *old = retired_.load(std::memory_order_relaxed);
        do
        {
            node->reclaim_next = old;
        } while (!retired_.compare_exchange_weak(
            old, node, std::memory_order_release, std::memory_order_relaxed));
    }

private:
    std::atomic<Node *> head_{nullptr};
    std::atomic<Node *> tail_{nullptr};

    // 运行期间弹出的旧 head 放这里，析构时统一 delete
    std::atomic<Node *> retired_{nullptr};
};

// ---- 简单并发测试 ----
int main()
{
    LockFreeQueue<int> q;

    constexpr int producer_n = 4;
    constexpr int consumer_n = 4;
    constexpr int per_producer = 50000;
    constexpr int total = producer_n * per_producer;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int p = 0; p < producer_n; ++p)
    {
        producers.emplace_back(
            [&q, &produced, p]()
            {
                int base = p * per_producer;
                for (int i = 0; i < per_producer; ++i)
                {
                    q.push(base + i);
                    produced.fetch_add(1, std::memory_order_relaxed);
                }
            });
    }

    for (int c = 0; c < consumer_n; ++c)
    {
        consumers.emplace_back(
            [&q, &consumed]()
            {
                int x;
                while (consumed.load(std::memory_order_relaxed) < total)
                    if (q.pop(x))
                        consumed.fetch_add(1, std::memory_order_relaxed);
                    else
                        std::this_thread::yield();
            });
    }

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();

    std::cout << "produced = " << produced.load() << "\n";
    std::cout << "consumed = " << consumed.load() << "\n";
    return 0;
}