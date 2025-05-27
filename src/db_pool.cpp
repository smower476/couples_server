#include "../include/db_pool.h"
#include <chrono>

using namespace std::chrono;

ConnectionPool::ConnectionPool(std::string conn_str, size_t pool_size)
    : conninfo(std::move(conn_str)), pool_size(pool_size) {
    for (size_t i = 0; i < pool_size; ++i) {
        pool.push(std::make_unique<TimedConnection>(std::make_unique<pqxx::connection>(conninfo)));
    }
    cleanup_thread = std::thread(&ConnectionPool::cleanup_idle_connections, this);
}

std::unique_ptr<pqxx::connection> ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mtx);
    cond.wait(lock, [&] { return !pool.empty(); });
    auto timed_conn = std::move(pool.front());
    pool.pop();
    // Проверяем, не устарело ли соединение
    auto now = steady_clock::now();
    if (duration_cast<minutes>(now - timed_conn->last_used).count() >= 5) {
        // Закрываем старое соединение и создаём новое
        timed_conn = std::make_unique<TimedConnection>(std::make_unique<pqxx::connection>(conninfo));
    }
    return std::move(timed_conn->conn);
}

void ConnectionPool::release(std::unique_ptr<pqxx::connection> conn) {
    std::lock_guard<std::mutex> lock(mtx);
    pool.push(std::make_unique<TimedConnection>(std::move(conn)));
    cond.notify_one();
}

void ConnectionPool::cleanup_idle_connections() {
    while (!stop_cleanup) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
        std::lock_guard<std::mutex> lock(mtx);
        size_t n = pool.size();
        std::vector<std::unique_ptr<TimedConnection>> keep;
        auto now = steady_clock::now();
        for (size_t i = 0; i < n; ++i) {
            auto timed_conn = std::move(pool.front());
            pool.pop();
            if (duration_cast<minutes>(now - timed_conn->last_used).count() < 5) {
                keep.push_back(std::move(timed_conn));
            } // иначе соединение уничтожается
        }
        for (auto& c : keep) pool.push(std::move(c));
    }
}

ConnectionPool::~ConnectionPool() {
    stop_cleanup = true;
    if (cleanup_thread.joinable()) cleanup_thread.join();
}

