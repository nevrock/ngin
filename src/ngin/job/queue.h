#ifndef THREADABLE_QUEUE_H
#define THREADABLE_QUEUE_H

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <map>
#include <string>

// --- Utility: Basic ThreadableQueue (Included here for self-containment if not elsewhere) ---
template<typename T>
class ThreadableQueue {
public:
    void push(T item) {
        { std::lock_guard<std::mutex> lock(m_mutex_); m_queue_.push(std::move(item)); }
        m_cv_.notify_one();
    }

    /**
     * @brief NEW: Tries to pop one item from the front of the queue.
     * @param item The item to be populated.
     * @return True if an item was successfully popped, false if the queue was empty.
     */
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        if (m_queue_.empty()) {
            return false;
        }
        item = std::move(m_queue_.front());
        m_queue_.pop();
        return true;
    }

    void pop_all(std::vector<T>& items) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        while (!m_queue_.empty()) { items.push_back(std::move(m_queue_.front())); m_queue_.pop(); }
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        return m_queue_.empty();
    }
    
    // NEW: Returns the current size of the queue.
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        return m_queue_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex_);
        std::queue<T> empty_queue;
        std::swap(m_queue_, empty_queue);
    }
private:
    std::queue<T> m_queue_;
    mutable std::mutex m_mutex_;
    std::condition_variable m_cv_;
};

#endif