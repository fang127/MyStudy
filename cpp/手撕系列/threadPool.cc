#include <iostream>
#include <future>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <atomic>
class ThreadPool
{
public:
    ThreadPool(std::size_t size_) : stop_(false)
    {
        for (size_t i = 0; i < size_; ++i)
        {
            threads_.emplace_back(
                [this]()
                {
                    while (true)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex_);
                            cond_.wait(lock,
                                       [this]()
                                       {
                                           return this->stop_ ||
                                                  !this->tasks_.empty();
                                       });
                            if (stop_ && tasks_.empty()) return;

                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        task();
                    }
                });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock lock(mutex_);
            stop_ = true;
        }
        cond_.notify_all();
        for (auto &t : threads_)
            if (t.joinable()) t.join();
    }

    template <typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using resultType = typename std::result_of<F(Args...)>::type;
        auto task =
            std::make_shared<std::packaged_task<resultType()>>(std::bind(
                std::forward<F>(f), std::forward<Args>(args)...)); // 修正这里

        std::future<resultType> res = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_)
                throw std::runtime_error(
                    "Now threadPool can't enqueue task, because threaPool is "
                    "stop");
            tasks_.emplace([task]() { (*task)(); });
        }
        cond_.notify_one();

        return res;
    }

private:
    std::mutex mutex_;
    std::queue<std::function<void()>> tasks_;
    std::condition_variable cond_;
    std::vector<std::thread> threads_;
    bool stop_;
};

int main()
{
    std::atomic<int> index = 0;
    ThreadPool pool(5);

    std::vector<std::future<void>> fs;
    fs.reserve(100000);

    for (int i = 0; i < 100000; ++i)
    {
        fs.emplace_back(pool.enqueue(
            [&]() { index.fetch_add(1, std::memory_order_relaxed); }));
    }

    for (auto &f : fs) f.get(); // 等待全部任务完成
    std::cout << index.load() << std::endl;
}