#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <pqxx/pqxx>

namespace yolo {

/// Thread-safe PostgreSQL connection pool using libpqxx
class DbPool {
public:
    struct Config {
        std::string host = "192.168.2.15";
        int port = 5432;
        std::string user = "maestro";
        std::string password;
        std::string database = "ai_context";
        int pool_size = 4;
    };

    explicit DbPool(const Config& config);
    ~DbPool();

    // Non-copyable, non-movable
    DbPool(const DbPool&) = delete;
    DbPool& operator=(const DbPool&) = delete;

    /// RAII connection guard - returns connection to pool on destruction
    class ConnectionGuard {
    public:
        ConnectionGuard(DbPool& pool, std::unique_ptr<pqxx::connection> conn);
        ~ConnectionGuard();

        ConnectionGuard(ConnectionGuard&&) noexcept;
        ConnectionGuard& operator=(ConnectionGuard&&) = delete;
        ConnectionGuard(const ConnectionGuard&) = delete;
        ConnectionGuard& operator=(const ConnectionGuard&) = delete;

        pqxx::connection& operator*() { return *conn_; }
        pqxx::connection* operator->() { return conn_.get(); }

    private:
        DbPool* pool_;
        std::unique_ptr<pqxx::connection> conn_;
    };

    /// Acquire a connection from the pool (blocks if none available)
    ConnectionGuard acquire();

    /// Get the connection string
    const std::string& connection_string() const { return conn_string_; }

    /// Get pool statistics
    struct Stats {
        int total_connections;
        int available_connections;
        int in_use_connections;
    };
    Stats stats() const;

private:
    void return_connection(std::unique_ptr<pqxx::connection> conn);
    std::unique_ptr<pqxx::connection> create_connection();

    std::string conn_string_;
    int pool_size_;
    std::queue<std::unique_ptr<pqxx::connection>> pool_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    int total_created_ = 0;
};

} // namespace yolo
