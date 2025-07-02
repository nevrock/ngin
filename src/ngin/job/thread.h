#ifndef NGIN_JOB_THREAD_H
#define NGIN_JOB_THREAD_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory> // For std::unique_ptr

// Assuming ngin::debug::Logger is available as per original JobSystem
#include <ngin/debug/logger.h>

namespace ngin {
namespace jobs {

class Thread {
public:
    // Constructor: Initializes the thread with an ID and the loop function.
    // The logger is passed to allow the thread to log its own lifecycle events.
    Thread(unsigned int id, std::function<void(unsigned int, std::atomic<bool>&, std::mutex&, std::condition_variable&)> thread_loop_func, ngin::debug::Logger* logger)
        : id_(id),
          stop_flag_(false),
          thread_loop_func_(std::move(thread_loop_func)),
          logger_(logger)
    {
        // Start the actual std::thread, passing the member function and 'this'
        // We pass the stop_flag_, mutex_, and cv_ to the loop function
        worker_thread_ = std::thread(&Thread::run, this);
        logger_->info("Thread " + std::to_string(id_) + " started.");
    }

    // Destructor: Ensures the thread is joined.
    ~Thread() {
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        logger_->info("Thread " + std::to_string(id_) + " destroyed.");
    }

    // Signals the thread to stop and notifies its condition variable.
    void signal_stop() {
        stop_flag_.store(true, std::memory_order_release);
        cv_.notify_all(); // Notify the thread to wake up and check stop flag
    }

    // Checks if the thread is joinable.
    bool joinable() const {
        return worker_thread_.joinable();
    }

    // Joins the underlying std::thread.
    void join() {
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    // Get the ID of the thread.
    unsigned int get_id() const {
        return id_;
    }

    // Accessors for the internal synchronization primitives,
    // allowing external JobSystem to interact with them for job distribution.
    std::mutex& get_mutex() { return mutex_; }
    std::condition_variable& get_condition_variable() { return cv_; }
    std::atomic<bool>& get_stop_flag() { return stop_flag_; }

private:
    // The actual entry point for the std::thread.
    // This will call the thread_loop_func_ provided in the constructor.
    void run() {
        if (thread_loop_func_) {
            // Pass this thread's ID to the loop function
            thread_loop_func_(id_, stop_flag_, mutex_, cv_);
        }
    }

    unsigned int id_;
    std::thread worker_thread_;
    std::atomic<bool> stop_flag_;
    std::mutex mutex_; // Mutex specific to this thread's operation (e.g., waiting for work)
    std::condition_variable cv_; // Condition variable specific to this thread
    std::function<void(unsigned int, std::atomic<bool>&, std::mutex&, std::condition_variable&)> thread_loop_func_;
    ngin::debug::Logger* logger_; // Pointer to the shared logger instance
};

} // namespace JobSystem
} // namespace Ngin

#endif // NGIN_JOB_THREAD_H