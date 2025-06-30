#ifndef JOB_SYSTEM_H
#define JOB_SYSTEM_H

// includes...
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
#include <sstream>
#include <iostream> 
#include <iomanip>  

#include <ngin/debug/logger.h>
#include <ngin/job/job.h>
#include <ngin/job/handle.h>
#include <ngin/job/thread.h> 
#include <ngin/job/queue.h>

// to_string helper...
static inline std::string to_string(JobType type) {
    switch (type) {
        case JobType::Physics: return "Physics";
        case JobType::Animation: return "Animation";
        case JobType::AI: return "AI";
        case JobType::TransformSync: return "TransformSync";
        case JobType::RenderCommandGeneration: return "RenderCommandGeneration";
        case JobType::AssetLoading: return "AssetLoading";
        case JobType::Other: return "Other";
        case JobType::None: return "None";
        default: return "Unknown";
    }
}
namespace ngin {
namespace jobs {

class JobSystem {
public:
    // NEW: A struct to hold a consistent snapshot of job counts.
    struct Snapshot {
        int total_pending = 0;
        std::map<JobType, int> type_counts;
    };

    // Constructor, Destructor, submit functions (no changes)
    JobSystem(unsigned int num_threads = std::thread::hardware_concurrency())
    : logger_(new Logger("JobSystem")),
      num_threads_(num_threads),
      total_pending_jobs_count_(0)
    {
        if (num_threads_ == 0) {
            num_threads_ = 1; 
        }
        logger_->info("JobSystem setup with " + std::to_string(num_threads_) + " threads");
        for (int i = static_cast<int>(JobType::None); i <= static_cast<int>(JobType::Other); ++i) {
            job_type_counts_[static_cast<JobType>(i)].store(0);
        }
        for (unsigned int i = 0; i < num_threads_; ++i) {
            worker_threads_.emplace_back(std::make_unique<ngin::jobs::Thread>(
                i,
                [this](unsigned int thread_id, std::atomic<bool>& stop_flag, std::mutex& mtx, std::condition_variable& cv) {
                    this->worker_thread_loop(thread_id, stop_flag, mtx, cv);
                },
                logger_
            ));
        }
    }
    ~JobSystem() { shutdown(); delete logger_; }
    void submit(std::function<void()> task_func, JobType type, JobHandle& handle) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            handle.get_counter()->fetch_add(1, std::memory_order_release);
            Job new_job(std::move(task_func), type, handle);
            job_queue_.push(std::move(new_job));
            job_type_counts_[type].fetch_add(1, std::memory_order_release);
            total_pending_jobs_count_.fetch_add(1, std::memory_order_release);
        }
        worker_cv_.notify_one();
    }
    JobHandle submit_jobs(std::vector<std::function<void()>> task_funcs, JobType type, const std::vector<JobHandle>& dependencies = {}) {
        JobHandle new_handle;
        for (const auto& dep_handle : dependencies) {
            wait_for(dep_handle);
        }
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            new_handle.get_counter()->fetch_add(task_funcs.size(), std::memory_order_release);
            for (auto& task_func : task_funcs) {
                Job new_job(std::move(task_func), type, new_handle);
                job_queue_.push(std::move(new_job));
                job_type_counts_[type].fetch_add(1, std::memory_order_release);
                total_pending_jobs_count_.fetch_add(1, std::memory_order_release);
            }
        }
        worker_cv_.notify_all();
        return new_handle;
    }

    // MODIFIED: wait_for now locks when decrementing counters.
    void wait_for(const JobHandle& handle) {
        if (handle.is_complete()) {
            return;
        }
        while (!handle.is_complete()) {
            Job job_to_execute;
            bool job_found = false;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                manager_cv_.wait(lock, [this, &handle]() {
                    return handle.is_complete() || !job_queue_.empty();
                });
                if (handle.is_complete()) break;
                if (!job_queue_.empty()) {
                    job_to_execute = std::move(job_queue_.front());
                    job_queue_.pop();
                    job_found = true;
                } else {
                     break; 
                }
            } 
            if (job_found) {
                job_to_execute.task();
                job_to_execute.handle.get_counter()->fetch_sub(1, std::memory_order_release);
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    job_type_counts_[job_to_execute.type].fetch_sub(1, std::memory_order_release);
                    total_pending_jobs_count_.fetch_sub(1, std::memory_order_release);
                }
                manager_cv_.notify_all();
            }
        }
    }
    
    // NEW: Returns a guaranteed consistent snapshot of the job system's state.
    Snapshot get_diagnostics_snapshot() const {
        Snapshot snapshot;
        std::lock_guard<std::mutex> lock(queue_mutex_);
        snapshot.total_pending = total_pending_jobs_count_.load(std::memory_order_relaxed);
        for (const auto& pair : job_type_counts_) {
            snapshot.type_counts[pair.first] = pair.second.load(std::memory_order_relaxed);
        }
        return snapshot;
    }

    // shutdown (no changes)
    void shutdown() {
        if (!worker_threads_.empty() && worker_threads_[0]->joinable()) {
            logger_->info("JobSystem: Shutting down workers...");
            for (const auto& worker_thread_ptr : worker_threads_) {
                worker_thread_ptr->signal_stop();
            }
            worker_cv_.notify_all();
            for (std::unique_ptr<ngin::jobs::Thread>& worker : worker_threads_) {
                if (worker->joinable()) {
                    worker->join();
                }
            }
            worker_threads_.clear();
            logger_->info("JobSystem: All workers shut down.");
        }
    }

private:
    // MODIFIED: worker_thread_loop now locks when decrementing counters.
    void worker_thread_loop(unsigned int thread_id, std::atomic<bool>& stop_flag, std::mutex& mtx, std::condition_variable& cv) {
        while (true) {
            Job job_to_execute;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                worker_cv_.wait(lock, [this, &stop_flag]() {
                    return !job_queue_.empty() || stop_flag.load(std::memory_order_acquire);
                });
                if (stop_flag.load(std::memory_order_acquire) && job_queue_.empty()) break;
                if (job_queue_.empty()) continue;
                job_to_execute = std::move(job_queue_.front());
                job_queue_.pop();
            }

            job_to_execute.task();
            job_to_execute.handle.get_counter()->fetch_sub(1, std::memory_order_release);
            
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                job_type_counts_[job_to_execute.type].fetch_sub(1, std::memory_order_release);
                total_pending_jobs_count_.fetch_sub(1, std::memory_order_release);
            }
            manager_cv_.notify_all();
        }
        logger_->info("Worker thread " + std::to_string(thread_id) + " exiting.");
    }

    // Member variables (no changes)
    Logger* logger_ = nullptr;
    unsigned int num_threads_;
    std::vector<std::unique_ptr<ngin::jobs::Thread>> worker_threads_;

    std::queue<Job> job_queue_;
    mutable std::mutex queue_mutex_;
    
    std::condition_variable worker_cv_;
    std::condition_variable manager_cv_;
    std::map<JobType, std::atomic<int>> job_type_counts_;
    std::atomic<int> total_pending_jobs_count_;
};
}
}

#endif 