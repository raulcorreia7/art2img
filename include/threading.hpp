#pragma once

#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace art2img {

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();
    
    // Non-copyable, movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    template<typename F>
    void enqueue(F&& task);
    
    void wait_all();
    size_t pending_tasks() const;
    
private:
    void worker_loop();
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_tasks_{0};
};

// Template implementation must be in header

template<typename F>
void ThreadPool::enqueue(F&& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            return;
        }
        tasks_.emplace(std::forward<F>(task));
    }
    condition_.notify_one();
}

} // namespace art2img