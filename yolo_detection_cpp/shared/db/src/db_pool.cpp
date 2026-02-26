#include "db_pool.h"
#include <spdlog/spdlog.h>
#include <sstream>

namespace yolo {

DbPool::DbPool(const Config& config) : pool_size_(config.pool_size) {
    std::ostringstream oss;
    oss << "host=" << config.host
        << " port=" << config.port
        << " user=" << config.user
        << " password=" << config.password
        << " dbname=" << config.database;
    conn_string_ = oss.str();

    spdlog::info("Initializing database pool (size={}) to {}:{}/{}",
                 pool_size_, config.host, config.port, config.database);

    // Pre-create connections
    for (int i = 0; i < pool_size_; ++i) {
        try {
            pool_.push(create_connection());
            ++total_created_;
        } catch (const std::exception& e) {
            spdlog::error("Failed to create DB connection {}/{}: {}", i + 1, pool_size_, e.what());
            // Continue - pool can work with fewer connections
        }
    }

    spdlog::info("Database pool initialized with {}/{} connections", total_created_, pool_size_);
}

DbPool::~DbPool() {
    std::lock_guard lock(mutex_);
    while (!pool_.empty()) {
        pool_.pop();
    }
}

std::unique_ptr<pqxx::connection> DbPool::create_connection() {
    auto conn = std::make_unique<pqxx::connection>(conn_string_);
    if (!conn->is_open()) {
        throw std::runtime_error("Failed to open database connection");
    }
    return conn;
}

DbPool::ConnectionGuard DbPool::acquire() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty(); });

    auto conn = std::move(pool_.front());
    pool_.pop();

    // Verify connection is still alive
    try {
        pqxx::nontransaction ntx(*conn);
        ntx.exec("SELECT 1");
    } catch (...) {
        spdlog::warn("Stale DB connection detected, reconnecting");
        try {
            conn = create_connection();
        } catch (const std::exception& e) {
            spdlog::error("Failed to reconnect: {}", e.what());
            throw;
        }
    }

    return ConnectionGuard(*this, std::move(conn));
}

void DbPool::return_connection(std::unique_ptr<pqxx::connection> conn) {
    {
        std::lock_guard lock(mutex_);
        pool_.push(std::move(conn));
    }
    cv_.notify_one();
}

DbPool::Stats DbPool::stats() const {
    std::lock_guard lock(mutex_);
    return Stats{
        .total_connections = total_created_,
        .available_connections = static_cast<int>(pool_.size()),
        .in_use_connections = total_created_ - static_cast<int>(pool_.size()),
    };
}

// ConnectionGuard implementation

DbPool::ConnectionGuard::ConnectionGuard(DbPool& pool, std::unique_ptr<pqxx::connection> conn)
    : pool_(&pool), conn_(std::move(conn)) {}

DbPool::ConnectionGuard::~ConnectionGuard() {
    if (conn_ && pool_) {
        pool_->return_connection(std::move(conn_));
    }
}

DbPool::ConnectionGuard::ConnectionGuard(ConnectionGuard&& other) noexcept
    : pool_(other.pool_), conn_(std::move(other.conn_)) {
    other.pool_ = nullptr;
}

} // namespace yolo
