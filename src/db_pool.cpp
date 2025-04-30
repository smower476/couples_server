#include "../include/db_pool.h"


ConnectionPool::ConnectionPool(std::string conn_str, size_t pool_size)
    : conninfo(std::move(conn_str)) {
    for (size_t i = 0; i < pool_size; ++i) {
        pool.push(std::make_unique<pqxx::connection>(conninfo));
    }
}

std::unique_ptr<pqxx::connection> ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mtx);
    cond.wait(lock, [&] { return !pool.empty(); });
    auto conn = std::move(pool.front());
    pool.pop();
    return conn;
}

void ConnectionPool::release(std::unique_ptr<pqxx::connection> conn) {
    std::lock_guard<std::mutex> lock(mtx);
    pool.push(std::move(conn));
    cond.notify_one();
}

