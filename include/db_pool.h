#ifndef DB_POOL_H
#define DB_POOL_H

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <vector>

struct TimedConnection {
    std::unique_ptr<pqxx::connection> conn;
    std::chrono::steady_clock::time_point last_used;
    TimedConnection(std::unique_ptr<pqxx::connection> c)
        : conn(std::move(c)), last_used(std::chrono::steady_clock::now()) {}
};

class ConnectionPool {
public:
    ConnectionPool(std::string conn_str, size_t pool_size);
    std::unique_ptr<pqxx::connection> acquire();
    void release(std::unique_ptr<pqxx::connection> conn);
    ~ConnectionPool();
private:
    void cleanup_idle_connections();
    std::string conninfo;
    std::queue<std::unique_ptr<TimedConnection>> pool;
    std::mutex mtx;
    std::condition_variable cond;
    bool stop_cleanup = false;
    std::thread cleanup_thread;
    size_t pool_size;
};

class ConnectionHandle {
public:
    ConnectionHandle(ConnectionPool &pool)
        : pool(pool), conn(pool.acquire()) {}

    pqxx::connection* get() const {
        return conn.get();
    }

    ~ConnectionHandle() {
        if (conn) {
            pool.release(std::move(conn));
        }
    }

private:
    ConnectionPool &pool;
    std::unique_ptr<pqxx::connection> conn;
};

#endif
