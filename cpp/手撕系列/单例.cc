#include <mutex>

// 懒汉单例模式示例
class Singleton1
{
public:
    // C++11 及以上版本中，局部静态变量的初始化是线程安全的
    static Singleton1 &getInstance()
    {
        static Singleton1 instance;
        return instance;
    }

private:
    Singleton1() {}
    Singleton1(const Singleton1 &) = delete;
    Singleton1 &operator=(const Singleton1 &) = delete;
};
std::mutex mutex_;
// 懒汉单例模式实现二：
class Singleton2
{
public:
    // 双重锁
    static Singleton2 &genInstance()
    {
        if (instance_ == nullptr)
        {
            // 加锁
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance_ == nullptr) instance_ = new Singleton2();
        }
        return *instance_;
    }

private:
    Singleton2() {}
    Singleton2(const Singleton2 &) = delete;
    Singleton2 &operator=(const Singleton2 &) = delete;

    static Singleton2 *instance_;
};

Singleton2 *Singleton2::instance_ = nullptr;

// 懒汉单例模式实现三：
class Singleton3
{
public:
    static Singleton3 &genInstance()
    {
        std::once_flag flag;
        std::call_once(flag, []() { instance_ = new Singleton3(); });
        return *instance_;
    }

private:
    Singleton3() {}
    Singleton3(const Singleton3 &) = delete;
    Singleton3 &operator=(const Singleton3 &) = delete;
    static Singleton3 *instance_;
};

Singleton3 *Singleton3::instance_ = nullptr;

// 饿汉单例模式示例
class Singleton4
{
public:
    // 在程序启动时就创建实例，线程安全
    static Singleton4 &getInstance() { return *instance_; }

private:
    Singleton4() {}
    Singleton4(const Singleton4 &) = delete;
    Singleton4 &operator=(const Singleton4 &) = delete;

    static Singleton4 *instance_;
};

Singleton4 *Singleton4::instance_ = new Singleton4();