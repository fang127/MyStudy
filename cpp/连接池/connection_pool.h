#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>

// 通用连接池接口，支持连接的创建、获取、释放、维护和统计功能。设计目标是高性能、线程安全和资源管理。
// 不绑定具体的连接类型，用户可以通过实现 IConnection
// 接口来适配不同的连接需求（如数据库连接、HTTP
// 连接等）。连接池会自动管理连接的生命周期，包括空闲连接的回收和泄漏连接的检测。用户可以通过配置参数来调整连接池的行为，以满足不同应用场景的需求。
class IConnection
{
public:
    virtual ~IConnection() = default;
    virtual bool connect() = 0;
    virtual void close() noexcept = 0;
    virtual bool ping() = 0;
};

class ConnectionPool : public std::enable_shared_from_this<ConnectionPool>
{
    struct PooledConnection;

public:
    /**
     * @brief         连接池的配置结构体，包含了连接池的各种参数设置。
     *
     */
    struct Config
    {
        // 最小连接数，连接池启动时会预创建这些连接以提高初始性能
        std::size_t minConnections = 2;
        // 最大连接数，连接池中允许存在的最大连接数量，超过这个数量的请求将被阻塞或失败
        std::size_t maxConnections = 16;
        // 获取连接的超时时间，如果在这个时间内无法获取到连接，acquire
        // 方法将返回 std::nullopt
        std::chrono::milliseconds acquireTimeout{1000};
        // 空闲连接的超时时间，如果连接在这个时间内没有被使用，将被回收
        std::chrono::milliseconds idleTimeout{30000};
        // 维护线程的间隔时间，连接池会定期检查和回收空闲或泄漏的连接
        std::chrono::milliseconds maintenanceInterval{1000};
        // 泄漏连接的超时时间，如果连接被借出后超过这个时间没有归还，连接池会认为它泄漏了，并尝试回收
        std::chrono::milliseconds leakTimeout{10000};
        // 连接重试的回退时间，如果连接失效，连接池会等待这个时间后尝试重新连接
        std::chrono::milliseconds reconnectBackoff{100};
        // 连接重试的最大次数，如果连接失效，连接池会尝试重新连接这个次数，超过后将放弃并记录失败
        int reconnectAttempts = 3;
    };

    /**
     * @brief         连接池的统计结构体，包含了连接池的各种统计数据
     *
     */
    struct Stats
    {
        // 总连接数，包括空闲和使用中的连接
        std::size_t totalConnections = 0;
        // 空闲连接数，当前在连接池中未被借出的连接数量
        std::size_t idleConnections = 0;
        // 使用中连接数，当前被借出但尚未归还的连接数量
        std::size_t inUseConnections = 0;
        // 泄漏连接数，当前被借出但超过 leakTimeout 没有归还的连接数量
        std::size_t leakedRecovered = 0;
        // 成功重连次数，连接池尝试重新连接失效连接时成功的次数
        std::size_t reconnectSuccess = 0;
        // 失败重连次数，连接池尝试重新连接失效连接时失败的次数
        std::size_t reconnectFail = 0;
        // 创建连接失败次数，连接池尝试创建新连接时失败的次数
        std::size_t createFail = 0;
    };

    /**
     * @brief
     * 是连接池中连接的智能指针封装，负责管理连接的生命周期和自动释放。用户通过
     * Handle 来访问连接，确保连接在使用完毕后能够正确返回池中或被销毁
     * 最关键的防泄漏设计！
     *
     */
    class Handle
    {
    public:
        Handle() = default;
        Handle(const Handle &) = delete;
        Handle &operator=(const Handle &) = delete;

        Handle(Handle &&other) noexcept;
        Handle &operator=(Handle &&other) noexcept;

        ~Handle();

        IConnection *operator->();
        IConnection &operator*();
        explicit operator bool() const noexcept;

        /**
         * @brief
         * 手动释放连接，将连接返回给连接池。通常情况下，用户不需要调用这个方法，因为
         * Handle
         * 的析构函数会自动释放连接。但是在某些特殊场景下（如需要提前释放连接以避免长时间占用资源），用户可以调用这个方法来手动释放连接
         *
         */
        void release();

    private:
        friend class ConnectionPool;

        Handle(std::weak_ptr<ConnectionPool> owner,
               std::shared_ptr<PooledConnection> conn);
        // 连接池的弱引用，避免循环引用，当连接池被销毁时，Handle
        // 可以检测到并避免访问已销毁的连接池
        std::weak_ptr<ConnectionPool> owner_;
        // 连接池中的连接的共享指针，确保连接在被 Handle
        // 使用时不会被销毁，同时允许连接池在必要时回收或销毁连接
        std::shared_ptr<PooledConnection> conn_;
    };

    using ConnectionFactory = std::function<std::unique_ptr<IConnection>()>;

    /**
     * @brief
     * 连接池的静态工厂方法，用于创建连接池实例。用户需要提供连接池的配置参数和一个连接工厂函数，连接工厂函数负责创建具体的连接对象（实现了
     * IConnection
     * 接口）。这个方法会返回一个共享指针，用户可以通过这个指针来管理连接池的生命周期
     *
     * @param         cfg
     * 连接池的配置参数，包含了连接池的各种设置，如最小连接数、最大连接数、超时时间等
     * @param         factory
     * 连接工厂函数，用于创建具体的连接对象。用户需要实现这个函数来返回一个实现了
     * IConnection 接口的连接对象，连接池会使用这个工厂函数来创建和管理连接
     * @return
     */
    static std::shared_ptr<ConnectionPool> Create(Config cfg,
                                                  ConnectionFactory factory);

    ~ConnectionPool();

    /**
     * @brief
     * 获取连接的方法，用户调用这个方法来从连接池中获取一个连接。方法会尝试在指定的超时时间内获取一个可用的连接，如果成功，返回一个
     * Handle
     * 对象，用户可以通过这个对象来访问连接。如果在超时时间内无法获取到连接，方法将返回
     * std::nullopt，表示获取连接失败。
     *
     * @param         timeout
     * 获取连接的超时时间，如果在这个时间内无法获取到连接，方法将返回
     * std::nullopt
     * @return
     */
    std::optional<Handle> acquire(std::chrono::milliseconds timeout);

    /**
     * @brief         获取连接的重载方法，使用连接池配置中的 acquireTimeout
     * 作为默认超时时间。用户可以直接调用这个方法来获取连接，而不需要每次都指定超时时间。
     *
     * @return
     */
    std::optional<Handle> acquire();

    /**
     * @brief         获取连接池的统计信息，返回一个 Stats
     * 结构体，包含了连接池的各种统计数据，如总连接数、空闲连接数、使用中连接数、泄漏连接数、重连成功次数、重连失败次数和创建连接失败次数等。用户可以通过这个方法来监控连接池的状态和性能，以便进行调优和故障排查。
     *
     * @return
     */
    Stats stats() const;

    /**
     * @brief
     * 关闭连接池，释放所有资源并停止维护线程。调用这个方法后，连接池将不再接受新的连接请求，并且所有现有的连接将被关闭和销毁。用户应该在应用程序结束时调用这个方法，以确保连接池能够正确清理资源并避免内存泄漏。
     *
     */
    void shutdown();

private:
    /**
     * @brief
     * 连接池内部使用的连接封装结构体，包含了连接的唯一标识符、连接对象的智能指针、连接的最后使用时间、连接被借出的时间以及连接的状态标志。这个结构体用于管理连接的生命周期和状态，支持连接池的维护和回收机制。
     *
     */
    struct PooledConnection
    {
        explicit PooledConnection(std::size_t idIn,
                                  std::unique_ptr<IConnection> c)
            : id(idIn), conn(std::move(c))
        {
        }

        // 连接的唯一标识符，便于追踪和管理连接
        std::size_t id;
        // 连接对象的智能指针，负责管理连接的生命周期，确保连接在不再使用时能够正确释放资源
        std::unique_ptr<IConnection> conn;
        // 连接的最后使用时间，用于判断连接是否空闲以及是否需要回收
        std::chrono::steady_clock::time_point lastUsed{
            std::chrono::steady_clock::now()};
        // 连接被借出的时间，用于判断连接是否泄漏以及是否需要回收
        std::chrono::steady_clock::time_point checkoutAt{
            std::chrono::steady_clock::now()};
        // 连接的状态标志，leased 表示连接是否被借出，retired
        // 表示连接是否被标记为废弃（如连接失效后不再使用）
        bool leased = false;
        bool retired = false;
    };

    ConnectionPool(Config cfg, ConnectionFactory factory);

    /**
     * @brief
     * 连接池的预热方法，在连接池启动时调用，用于预创建一定数量的连接以提高初始性能。根据配置中的
     * minConnections
     * 参数，连接池会尝试创建并连接这些连接，以确保在应用程序开始处理请求时能够快速提供可用的连接。如果预热过程中发生错误（如连接创建失败），连接池会记录失败次数，并继续尝试创建剩余的连接，直到达到
     * minConnections 或者遇到不可恢复的错误为止。
     *
     * @return
     * @return
     */
    bool warmup();

    /**
     * @brief
     * 连接池的连接创建和连接方法，用于创建一个新的连接并尝试连接。方法会使用连接工厂函数来创建一个新的连接对象，并调用它的
     * connect 方法来建立连接。如果连接成功，方法将返回一个包含新连接的
     * PooledConnection
     * 对象的共享指针；如果连接失败，方法将记录失败次数，并返回一个空的共享指针。这个方法在连接池需要创建新连接时被调用，如在预热阶段或当现有连接不足以满足请求时。
     *
     * @param         id
     * @return
     */
    std::shared_ptr<PooledConnection> createAndConnect(std::size_t id);

    /**
     * @brief
     * 连接池的连接健康检查方法，用于确保连接的健康状态。方法会调用连接对象的ping方法来检查连接是否仍然有效。如果连接不健康，方法会尝试重新连接指定次数（根据配置中的reconnectAttempts参数），每次重试之间会有一个回退时间（根据配置中的reconnectBackoff参数）。如果重试成功，方法将更新连接的状态并返回true；如果重试失败，方法将标记连接为废弃，并返回false。这个方法在获取连接时被调用，以确保提供给用户的连接是健康的。
     *
     * @param         c
     * @return
     * @return
     */
    bool ensureHealthy(const std::shared_ptr<PooledConnection> &c);

    /**
     * @brief
     * 连接池的连接释放方法，用于将连接返回给连接池。方法会更新连接的状态，将其标记为未被借出，并将其放回空闲连接队列中，以便其他请求可以使用这个连接。如果连接被标记为废弃（如连接失效后不再使用），方法会销毁连接而不是放回池中。这个方法在Handle的析构函数或用户调用Handle::release时被调用，以确保连接能够正确返回池中或被销毁，避免资源泄漏。
     *
     * @param         c
     */
    void releaseConnection(const std::shared_ptr<PooledConnection> &c);

    /**
     * @brief
     * 连接池的连接销毁方法，用于销毁一个连接并释放其资源。方法会调用连接对象的close方法来关闭连接，并将连接从连接池的管理中移除。如果连接被标记为废弃（如连接失效后不再使用），方法会确保连接被正确销毁以释放资源。这个方法在维护线程中被调用，以回收空闲或泄漏的连接，以及在Handle的析构函数或用户调用Handle::release时被调用，以确保连接能够正确销毁，避免资源泄漏。
     *
     * @param         c
     */
    void destroyConnection(const std::shared_ptr<PooledConnection> &c);

    /**
     * @brief
     * 连接池的维护循环方法，在一个独立的线程中运行，定期检查和回收空闲或泄漏的连接。方法会根据配置中的maintenanceInterval参数来控制检查的频率，在每次检查时会调用reclaimIdleAndLeakedLocked方法来回收空闲或泄漏的连接。维护循环会持续运行，直到连接池被关闭（调用shutdown方法）。这个方法确保连接池能够自动管理连接的生命周期，避免资源泄漏和性能问题。
     *
     */
    void maintenanceLoop();

    /**
     * @brief
     * 连接池的连接回收方法，用于回收空闲或泄漏的连接。方法会遍历连接池中的空闲连接队列，检查每个连接的最后使用时间，如果连接在配置中的idleTimeout时间内没有被使用，将被回收（销毁）。同时，方法还会检查所有被借出的连接，如果某个连接被借出后超过配置中的leakTimeout时间没有归还，连接池会认为它泄漏了，并尝试回收（销毁）这个连接。这个方法在维护循环中被调用，以确保连接池能够自动管理连接的生命周期，避免资源泄漏和性能问题。
     *
     * @param         now
     */
    void reclaimIdleAndLeakedLocked(std::chrono::steady_clock::time_point now);

private:
    Config cfg_;
    // 工厂函数：创建具体连接
    ConnectionFactory factory_;

    mutable std::mutex mu_;
    std::condition_variable cv_;
    bool stopping_ = false;

    // 连接ID生成器，确保每个连接都有一个唯一的标识符，便于追踪和管理连接
    std::size_t nextId_ = 1;
    // 当前连接池中存在的连接总数，包括空闲和使用中的连接
    std::size_t totalConnections_ = 0;
    // 当前被借出但超过 leakTimeout
    // 没有归还的连接数量，这些连接被认为是泄漏了，并且已经被回收（销毁）
    std::size_t leakedRecovered_ = 0;

    // 连接重试的统计数据，记录连接池尝试重新连接失效连接时成功和失败的次数，以及创建连接失败的次数。
    std::size_t reconnectSuccess_ = 0;
    std::size_t reconnectFail_ = 0;
    std::size_t createFail_ = 0;

    // 空闲连接队列（复用核心）
    std::deque<std::shared_ptr<PooledConnection>> idle_;
    // 使用中连接
    std::unordered_map<std::size_t, std::shared_ptr<PooledConnection>> inUse_;

    // 维护线程
    std::thread maintenanceThread_;
};
