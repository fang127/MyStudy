#include <mutex>
#include <thread>
#include <iostream>

// 必定死锁的代码示例
std::mutex mutexA;
std::mutex mutexB;

void thread1()
{
    std::lock_guard<std::mutex> lockA(mutexA);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作
    std::cout << "Thread 1 acquired mutexA" << std::endl;
    std::lock_guard<std::mutex> lockB(mutexB); // 线程1等待线程2释放mutexB
    std::cout << "Thread 1 acquired mutexB" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作
    std::cout << "Thread 1 released mutexA and mutexB" << std::endl;
}

void thread2()
{
    std::lock_guard<std::mutex> lockB(mutexB);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作
    std::cout << "Thread 2 acquired mutexB" << std::endl;
    std::lock_guard<std::mutex> lockA(mutexA); // 线程2等待线程1释放mutexA
    std::cout << "Thread 2 acquired mutexA" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作
    std::cout << "Thread 2 released mutexB and mutexA" << std::endl;
}

int main()
{
    std::thread t1(thread1);
    std::thread t2(thread2);

    t1.join();
    t2.join();

    return 0;
}