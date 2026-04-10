#include <atomic>
#include <iostream>

template <typename T>
class mySharedPtr
{
public:
    explicit mySharedPtr(T *data = nullptr) : data_(data)
    {
        count_ = data_ ? new std::atomic<int>(1) : nullptr;
    }

    ~mySharedPtr() { release(); }

    mySharedPtr(const mySharedPtr &oth) : data_(oth.data_), count_(oth.count_)
    {
        if (count_) count_->fetch_add(1, std::memory_order_relaxed);
    }

    mySharedPtr<T> &operator=(const mySharedPtr<T> &oth)
    {
        if (this == &oth) return *this;
        release();
        data_ = oth.data_;
        count_ = oth.count_;
        if (count_) count_->fetch_add(1, std::memory_order_relaxed);
        return *this;
    }

    int count() const
    {
        return count_ ? count_->load(std::memory_order_relaxed) : 0;
    }

    T *operator->() { return this->data_; }
    const T *operator->() const { return this->data_; }

    T &operator*() { return (*data_); }
    const T &operator*() const { return (*data_); }

private:
    void release()
    {
        if (!count_) return;
        // fetch_sub(1) 的返回值是减之前的值，不是减之后的值。
        if (count_->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete data_;
            delete count_;
        }
        data_ = nullptr;
        count_ = nullptr;
    }

    T *data_;
    std::atomic<int> *count_;
};

// 测试共享指针
void testSharedPtr()
{
    mySharedPtr<int> ptr1(new int(10));
    std::cout << "ptr1 count: " << ptr1.count() << std::endl; // 输出: 1

    {
        mySharedPtr<int> ptr2 = ptr1; // 共享ptr1的资源
        std::cout << "ptr2 count: " << ptr2.count() << std::endl; // 输出: 2
        std::cout << "ptr2 value: " << *ptr2 << std::endl;        // 输出: 10
    }

    std::cout << "ptr1 count after ptr2 goes out of scope: " << ptr1.count()
              << std::endl; // 输出: 1
}

int main()
{
    testSharedPtr();
    return 0;
}