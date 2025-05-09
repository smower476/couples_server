#ifndef DB_POOL_H
#define DB_POOL_H

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

class ConnectionPool {
public:
    ConnectionPool(std::string conn_str, size_t pool_size);
    std::unique_ptr<pqxx::connection> acquire();
    void release(std::unique_ptr<pqxx::connection> conn);

private:
    std::string conninfo;
    std::queue<std::unique_ptr<pqxx::connection>> pool;
    std::mutex mtx;
    std::condition_variable cond;
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
