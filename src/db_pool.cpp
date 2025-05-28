#include "../include/db_pool.h"
#include <chrono>

using namespace std::chrono;

ConnectionPool::ConnectionPool(std::string conn_str, size_t pool_size)
    : conninfo(std::move(conn_str)), pool_size(pool_size) {
    // Initialize with just one connection to ensure the pool isn't empty
    pool.push(std::make_unique<TimedConnection>(std::make_unique<pqxx::connection>(conninfo)));
    cleanup_thread = std::thread(&ConnectionPool::cleanup_idle_connections, this);
}

std::unique_ptr<pqxx::connection> ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Always create a fresh connection - this ensures we don't use expired connections
    auto fresh_connection = std::make_unique<pqxx::connection>(conninfo);
    
    // If the pool is not at maximum capacity, we'll release a connection to it later
    if (pool.size() < pool_size) {
        // Add a new connection to the pool if it's below capacity
        pool.push(std::make_unique<TimedConnection>(std::make_unique<pqxx::connection>(conninfo)));
        cond.notify_one();
    } else if (!pool.empty()) {
        // If the pool is at capacity and not empty, remove the oldest connection
        pool.pop();
        // And add a fresh one (done in the release method when this connection is released)
    }
    
    return fresh_connection;
}

void ConnectionPool::release(std::unique_ptr<pqxx::connection> conn) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Only add the connection to the pool if we're below capacity
    if (pool.size() < pool_size) {
        pool.push(std::make_unique<TimedConnection>(std::move(conn)));
        cond.notify_one();
    }
    // Otherwise, let the connection be destroyed
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
            if (duration_cast<minutes>(now - timed_conn->last_used).count() < 0.5) {
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

