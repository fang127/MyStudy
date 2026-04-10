#include "connection_pool.h"

#include <algorithm>
#include <stdexcept>
#include <thread>

ConnectionPool::Handle::Handle(std::weak_ptr<ConnectionPool> owner,
                               std::shared_ptr<PooledConnection> conn)
    : owner_(std::move(owner)), conn_(std::move(conn))
{
}

ConnectionPool::Handle::Handle(Handle &&other) noexcept
    : owner_(std::move(other.owner_)), conn_(std::move(other.conn_))
{
}

ConnectionPool::Handle &ConnectionPool::Handle::operator=(
    Handle &&other) noexcept
{
    if (this != &other)
    {
        release();
        owner_ = std::move(other.owner_);
        conn_ = std::move(other.conn_);
    }
    return *this;
}

ConnectionPool::Handle::~Handle() { release(); }

IConnection *ConnectionPool::Handle::operator->()
{
    return conn_ ? conn_->conn.get() : nullptr;
}

IConnection &ConnectionPool::Handle::operator*() { return *(conn_->conn); }

ConnectionPool::Handle::operator bool() const noexcept
{
    return conn_ && conn_->conn;
}

void ConnectionPool::Handle::release()
{
    if (!conn_) return;
    if (auto owner = owner_.lock()) owner->releaseConnection(conn_);
    conn_.reset();
}

std::shared_ptr<ConnectionPool> ConnectionPool::Create(
    Config cfg, ConnectionFactory factory)
{
    if (!factory) throw std::invalid_argument("factory must not be empty");
    if (cfg.minConnections > cfg.maxConnections)
        throw std::invalid_argument("minConnections must be <= maxConnections");
    auto p = std::shared_ptr<ConnectionPool>(
        new ConnectionPool(std::move(cfg), std::move(factory)));
    if (!p->warmup()) throw std::runtime_error("connection pool warmup failed");
    p->maintenanceThread_ = std::thread([p]() { p->maintenanceLoop(); });
    return p;
}

ConnectionPool::ConnectionPool(Config cfg, ConnectionFactory factory)
    : cfg_(std::move(cfg)), factory_(std::move(factory))
{
}

ConnectionPool::~ConnectionPool() { shutdown(); }

bool ConnectionPool::warmup()
{
    for (std::size_t i = 0; i < cfg_.minConnections; ++i)
    {
        std::size_t id = 0;
        {
            std::lock_guard<std::mutex> lk(mu_);
            id = nextId_++;
        }

        auto c = createAndConnect(id);
        if (!c) return false;

        std::lock_guard<std::mutex> lk(mu_);
        idle_.push_back(c);
        ++totalConnections_;
    }
    return true;
}

std::shared_ptr<ConnectionPool::PooledConnection>
ConnectionPool::createAndConnect(std::size_t id)
{
    for (int attempt = 0; attempt < std::max(1, cfg_.reconnectAttempts);
         ++attempt)
    {
        auto raw = factory_();
        if (!raw) continue;
        bool ok = false;
        try
        {
            ok = raw->connect();
        }
        catch (...)
        {
            ok = false;
        }

        if (ok)
        {
            if (attempt > 0)
            {
                std::lock_guard<std::mutex> lk(mu_);
                ++reconnectSuccess_;
            }
            return std::make_shared<PooledConnection>(id, std::move(raw));
        }

        try
        {
            raw->close();
        }
        catch (...)
        {
        }
        if (attempt + 1 < std::max(1, cfg_.reconnectAttempts))
            std::this_thread::sleep_for(cfg_.reconnectBackoff);
    }

    std::lock_guard<std::mutex> lk(mu_);
    ++reconnectFail_;
    ++createFail_;
    return nullptr;
}

bool ConnectionPool::ensureHealthy(const std::shared_ptr<PooledConnection> &c)
{
    if (!c || !c->conn) return false;
    bool healthy = false;
    try
    {
        healthy = c->conn->ping();
    }
    catch (...)
    {
        healthy = false;
    }

    if (healthy) return true;

    try
    {
        c->conn->close();
    }
    catch (...)
    {
    }

    // 如果连接不健康，尝试重连，重连成功则继续使用该连接，否则返回失败
    for (int attempt = 0; attempt < std::max(1, cfg_.reconnectAttempts);
         ++attempt)
    {
        bool ok = false;
        try
        {
            ok = c->conn->connect();
        }
        catch (...)
        {
            ok = false;
        }
        if (ok)
        {
            std::lock_guard<std::mutex> lk(mu_);
            ++reconnectSuccess_;
            return true;
        }
        if (attempt + 1 < std::max(1, cfg_.reconnectAttempts))
            std::this_thread::sleep_for(cfg_.reconnectBackoff);
    }

    std::lock_guard<std::mutex> lk(mu_);
    ++reconnectFail_;
    return false;
}

std::optional<ConnectionPool::Handle> ConnectionPool::acquire(
    std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true)
    {
        std::shared_ptr<PooledConnection> chosen;
        std::size_t createId = 0;
        bool shouldCreate = false;

        {
            std::unique_lock<std::mutex> lk(mu_);
            if (stopping_) return std::nullopt;

            if (!idle_.empty())
            {
                chosen = idle_.front();
                idle_.pop_front();
                chosen->leased = true;
                chosen->checkoutAt = std::chrono::steady_clock::now();
                inUse_[chosen->id] = chosen;
            }
            else if (totalConnections_ < cfg_.maxConnections)
            {
                shouldCreate = true;
                createId = nextId_++;
                ++totalConnections_;
            }
            else
            {
                if (cv_.wait_until(lk, deadline) == std::cv_status::timeout)
                    return std::nullopt;
                continue;
            }
        }

        // 如果从空闲连接中选中了一个连接，先检查它是否健康，如果不健康则销毁它并继续循环尝试获取连接
        if (chosen)
        {
            if (!ensureHealthy(chosen))
            {
                destroyConnection(chosen);
                continue;
            }
            return Handle{weak_from_this(), chosen};
        }

        if (shouldCreate)
        {
            auto created = createAndConnect(createId);
            if (!created)
            {
                std::lock_guard<std::mutex> lk(mu_);
                if (totalConnections_ > 0) --totalConnections_;
                cv_.notify_one();

                if (std::chrono::steady_clock::now() >= deadline)
                    return std::nullopt;
                continue;
            }

            {
                std::lock_guard<std::mutex> lk(mu_);
                if (stopping_)
                {
                    if (totalConnections_ > 0) --totalConnections_;
                    try
                    {
                        created->conn->close();
                    }
                    catch (...)
                    {
                    }
                    return std::nullopt;
                }
                created->leased = true;
                created->checkoutAt = std::chrono::steady_clock::now();
                inUse_[created->id] = created;
            }

            if (!ensureHealthy(created))
            {
                destroyConnection(created);
                continue;
            }
            return Handle{weak_from_this(), created};
        }
    }
}

std::optional<ConnectionPool::Handle> ConnectionPool::acquire()
{
    return acquire(cfg_.acquireTimeout);
}

void ConnectionPool::releaseConnection(
    const std::shared_ptr<PooledConnection> &c)
{
    if (!c) return;

    bool notify = false;
    bool shouldDestroy = false;
    {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = inUse_.find(c->id);
        if (it == inUse_.end()) return;
        inUse_.erase(it);

        c->leased = false;
        c->lastUsed = std::chrono::steady_clock::now();

        if (stopping_ || c->retired)
        {
            shouldDestroy = true;
            if (totalConnections_ > 0) --totalConnections_;
        }
        else
        {
            idle_.push_back(c);
            notify = true;
        }
    }

    if (!shouldDestroy)
    {
        if (!ensureHealthy(c))
        {
            destroyConnection(c);
            return;
        }
    }
    else
    {
        try
        {
            c->conn->close();
        }
        catch (...)
        {
        }
    }

    if (notify) cv_.notify_one();
}

void ConnectionPool::destroyConnection(
    const std::shared_ptr<PooledConnection> &c)
{
    if (!c) return;

    {
        std::lock_guard<std::mutex> lk(mu_);
        inUse_.erase(c->id);

        auto eraseIt =
            std::find_if(idle_.begin(), idle_.end(),
                         [&](const std::shared_ptr<PooledConnection> &p)
                         { return p && p->id == c->id; });
        if (eraseIt != idle_.end()) idle_.erase(eraseIt);

        c->retired = true;
        if (totalConnections_ > 0) --totalConnections_;
    }

    try
    {
        c->conn->close();
    }
    catch (...)
    {
    }

    cv_.notify_one();
}

void ConnectionPool::reclaimIdleAndLeakedLocked(
    std::chrono::steady_clock::time_point now)
{
    // 回收空闲连接：遍历 idle_ 列表，检查每个连接的空闲时间，如果超过了
    // cfg_.idleTimeout 且当前总连接数超过了
    // cfg_.minConnections，则将该连接标记为 retired，并尝试关闭它，同时从 idle_
    // 列表中移除它
    for (auto it = idle_.begin(); it != idle_.end();)
    {
        auto &c = *it;
        if (!c)
        {
            it = idle_.erase(it);
            continue;
        }
        const auto idleFor = now - c->lastUsed;
        if (totalConnections_ > cfg_.minConnections &&
            idleFor >= cfg_.idleTimeout)
        {
            c->retired = true;
            try
            {
                c->conn->close();
            }
            catch (...)
            {
            }
            if (totalConnections_ > 0) --totalConnections_;
            it = idle_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 回收泄漏连接：遍历 inUse_ 列表，检查每个连接的租用时间，如果超过了
    // cfg_.leakTimeout，则将该连接标记为 retired，并尝试关闭它，同时从 inUse_
    // 列表中移除它，并增加 leakedRecovered_ 计数
    for (auto it = inUse_.begin(); it != inUse_.end();)
    {
        auto &c = it->second;
        if (!c)
        {
            it = inUse_.erase(it);
            continue;
        }
        const auto leasedFor = now - c->checkoutAt;
        if (leasedFor >= cfg_.leakTimeout)
        {
            c->retired = true;
            if (totalConnections_ > 0) --totalConnections_;
            ++leakedRecovered_;
            it = inUse_.erase(it);
            cv_.notify_one();
        }
        else
        {
            ++it;
        }
    }
}

void ConnectionPool::maintenanceLoop()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lk(mu_);
            // 只有在停止标志被设置时才会退出循环，否则每隔 maintenanceInterval
            // 进行一次维护检查
            if (cv_.wait_for(lk, cfg_.maintenanceInterval,
                             [&]() { return stopping_; }))
            {
                break;
            }
            // 在每次维护检查时，首先调用 reclaimIdleAndLeakedLocked
            // 来回收空闲连接和泄漏连接
            reclaimIdleAndLeakedLocked(std::chrono::steady_clock::now());
        }

        while (true)
        {
            std::size_t id = 0;
            {
                std::lock_guard<std::mutex> lk(mu_);
                // 在回收空闲连接和泄漏连接之后，如果当前总连接数小于
                // cfg_.minConnections， 则继续创建新的连接来补充池子，直到达到
                // cfg_.minConnections 或者停止标志被设置
                if (stopping_ || totalConnections_ >= cfg_.minConnections)
                    break;
                id = nextId_++;
                ++totalConnections_;
            }

            auto c = createAndConnect(id);
            if (!c)
            {
                std::lock_guard<std::mutex> lk(mu_);
                if (totalConnections_ > 0) --totalConnections_;
                break;
            }

            {
                std::lock_guard<std::mutex> lk(mu_);
                if (stopping_)
                {
                    if (totalConnections_ > 0) --totalConnections_;
                    try
                    {
                        c->conn->close();
                    }
                    catch (...)
                    {
                    }
                    break;
                }
                idle_.push_back(c);
                cv_.notify_one();
            }
        }
    }

    // 在维护线程退出之前，先将所有连接都标记为
    // retired，并尝试关闭它们，同时清空 idle_ 和 inUse_ 列表，并将
    // totalConnections_ 重置为 0
    std::deque<std::shared_ptr<PooledConnection>> idleToClose;
    std::unordered_map<std::size_t, std::shared_ptr<PooledConnection>>
        inUseToRetire;

    {
        std::lock_guard<std::mutex> lk(mu_);
        idleToClose.swap(idle_);
        inUseToRetire.swap(inUse_);
        totalConnections_ = 0;
    }

    for (auto &c : idleToClose)
    {
        if (!c) continue;
        try
        {
            c->conn->close();
        }
        catch (...)
        {
        }
    }
    for (auto &kv : inUseToRetire)
    {
        auto &c = kv.second;
        if (!c) continue;
        c->retired = true;
        try
        {
            c->conn->close();
        }
        catch (...)
        {
        }
    }
}

ConnectionPool::Stats ConnectionPool::stats() const
{
    std::lock_guard<std::mutex> lk(mu_);
    Stats s;
    s.totalConnections = totalConnections_;
    s.idleConnections = idle_.size();
    s.inUseConnections = inUse_.size();
    s.leakedRecovered = leakedRecovered_;
    s.reconnectSuccess = reconnectSuccess_;
    s.reconnectFail = reconnectFail_;
    s.createFail = createFail_;
    return s;
}

void ConnectionPool::shutdown()
{
    {
        std::lock_guard<std::mutex> lk(mu_);
        if (stopping_) return;
        stopping_ = true;
    }
    cv_.notify_all();

    if (maintenanceThread_.joinable()) maintenanceThread_.join();
}
