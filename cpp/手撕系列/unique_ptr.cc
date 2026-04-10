#include <iostream>

template <typename T>
class MyUniquePtr
{
public:
    explicit MyUniquePtr(T *ptr = nullptr) : ptr_(ptr) {}
    ~MyUniquePtr() { delete ptr_; }
    // 删除复制构造函数和复制赋值运算符
    MyUniquePtr(const MyUniquePtr &) = delete;
    MyUniquePtr &operator=(const MyUniquePtr &) = delete;
    // 提供移动构造函数和移动赋值运算符
    MyUniquePtr(MyUniquePtr &&other) noexcept
    {
        if (this != &other)
        {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
    }

    MyUniquePtr &operator=(MyUniquePtr &&other) noexcept
    {
        if (this != &other)
        {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    T *operator->() { return ptr_; }
    const T *operator->() const { return ptr_; }
    T &operator*() { return *ptr_; }
    const T &operator*() const { return *ptr_; }

private:
    T *ptr_;
};