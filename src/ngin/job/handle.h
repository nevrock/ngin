#ifndef JOB_HANDLE_H
#define JOB_HANDLE_H 

#include <iostream>
#include <string>
#include <sstream> // Required for std::stringstream
#include <vector>
#include <tuple>
#include <memory>
#include <atomic>

#include <ngin/debug/logger.h>

class JobHandle {

public:
    JobHandle() : counter_(std::make_shared<std::atomic<int>>(0)) {
    }
    ~JobHandle() {
    }

    bool is_complete() const {
        return counter_->load(std::memory_order_acquire) == 0;
    }

    // This version is for non-const JobHandle objects
    std::shared_ptr<std::atomic<int>>& get_counter() {
        return counter_;
    }

    // ADDED: A const-overload for const JobHandle objects (like in wait_for)
    const std::shared_ptr<std::atomic<int>>& get_counter() const {
        return counter_;
    }


private:
    Logger* logger_ = nullptr;
    std::shared_ptr<std::atomic<int>> counter_; 

};

#endif