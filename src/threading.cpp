#include "threading.hpp"

namespace art2image {

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) {
            num_threads = 1; // Fallback to single thread
        }
    }
    
    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker_loop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
            ++active_tasks_;
        }
        
        try {
            task();
        } catch (...) {
            // Log error? For now, just continue
        }
        
        --active_tasks_;
    }
}

void ThreadPool::wait_all() {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (tasks_.empty() && active_tasks_ == 0) {
            break;
        }
        lock.unlock();
        std::this_thread::yield();
    }
}

size_t ThreadPool::pending_tasks() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size() + active_tasks_;
}

} // namespace art2image