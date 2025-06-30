#ifndef JOB_SYSTEM_STEALER_H
#define JOB_SYSTEM_STEALER_H

#include <vector>
#include <deque>
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
#include <utility>
#include <numeric> // For std::iota
#include <algorithm> // For std::remove

#include <ngin/debug/logger.h>
#include <ngin/job/job.h>
#include <ngin/job/handle.h>
#include <ngin/job/thread.h> 

// Static helper function to convert JobType enum to a string for logging
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
    // A struct to hold a consistent snapshot of job counts for diagnostics.
    struct Snapshot {
        int total_pending = 0;
        std::map<JobType, int> type_counts;
    };
    // A struct to hold a snapshot of a single thread's queue for diagnostics.
    struct ThreadDiagnostics {
        unsigned int thread_id;
        size_t job_count;
        std::map<JobType, int> type_counts;
        std::string dedication_type = "General"; // Default to general
    };

    /**
     * @brief Constructs the JobSystem.
     * @param num_threads The number of worker threads to create. Defaults to the hardware concurrency.
     */
    JobSystem(unsigned int num_threads = std::thread::hardware_concurrency())
        : logger_(new Logger("JobSystem")),
          num_threads_(num_threads),
          total_pending_jobs_count_(0),
          submit_thread_index_(0),
          per_queue_mutexes_(num_threads)
    {
        if (num_threads_ == 0) {
            num_threads_ = 1; 
        }
        logger_->info("JobSystem setup with " + std::to_string(num_threads_) + " threads");

        for (int i = static_cast<int>(JobType::None); i <= static_cast<int>(JobType::Other); ++i) {
            job_type_counts_[static_cast<JobType>(i)].store(0);
        }
        
        per_thread_queues_.resize(num_threads_);
        
        // Initially, all threads are general-purpose
        general_purpose_thread_indices_.resize(num_threads_);
        std::iota(general_purpose_thread_indices_.begin(), general_purpose_thread_indices_.end(), 0);

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

    /**
     * @brief Destructor. Shuts down the job system and joins all worker threads.
     */
    ~JobSystem() {
        shutdown();
        delete logger_;
    }


    void dedicate_thread_to_job_type(JobType type, unsigned int thread_index) {
        if (thread_index >= num_threads_) {
            logger_->warn("Attempted to dedicate an invalid thread index: " + std::to_string(thread_index));
            return;
        }
        
        std::lock_guard<std::mutex> lock(dedication_mutex_);
        
        // Store the dedication
        thread_dedications_[type] = thread_index;
        
        // Remove this thread from the pool of general purpose threads if it's there
        auto& indices = general_purpose_thread_indices_;
        indices.erase(std::remove(indices.begin(), indices.end(), thread_index), indices.end());
        
        logger_->info("Thread " + std::to_string(thread_index) + " dedicated to " + to_string(type) + " jobs.");
    }

    /**
     * @brief Submits a single job to the system.
     * The job is assigned to a worker queue in a round-robin fashion.
     * @param task_func The function to execute.
     * @param type The type of the job.
     * @param handle The handle this job will contribute to.
     */
    void submit(std::function<void()> task_func, JobType type, JobHandle& handle) {
        handle.get_counter()->fetch_add(1, std::memory_order_release);
        Job new_job(std::move(task_func), type, handle);

        job_type_counts_[type].fetch_add(1, std::memory_order_release);
        total_pending_jobs_count_.fetch_add(1, std::memory_order_release);
        
        unsigned int queue_index;
        
        // Check for a dedicated thread first
        std::lock_guard<std::mutex> lock(dedication_mutex_);
        auto it = thread_dedications_.find(type);
        if (it != thread_dedications_.end()) {
            queue_index = it->second;
        } else {
            // Otherwise, assign to a general-purpose thread round-robin
            if (general_purpose_thread_indices_.empty()) {
                // Fallback if all threads are dedicated, just use regular round-robin on all threads
                 queue_index = submit_thread_index_.fetch_add(1) % num_threads_;
            } else {
                unsigned int general_purpose_pool_index = submit_thread_index_.fetch_add(1) % general_purpose_thread_indices_.size();
                queue_index = general_purpose_thread_indices_[general_purpose_pool_index];
            }
        }
        
        push_to_queue(queue_index, std::move(new_job));

        worker_cv_.notify_one();
    }

    /**
     * @brief Submits a batch of jobs that all contribute to a single new handle.
     * @param task_funcs A vector of functions to execute.
     * @param type The type of all jobs in this batch.
     * @param dependencies A vector of handles that must complete before these jobs can start.
     * @return A new JobHandle that represents this batch of work.
     */
    JobHandle submit_jobs(const std::vector<std::function<void()>>& task_funcs, JobType type, const std::vector<JobHandle>& dependencies = {}) {
        JobHandle new_handle;
        for (const auto& dep_handle : dependencies) {
            wait_for(dep_handle);
        }

        size_t num_jobs = task_funcs.size();
        new_handle.get_counter()->fetch_add(num_jobs, std::memory_order_release);
        job_type_counts_[type].fetch_add(num_jobs, std::memory_order_release);
        total_pending_jobs_count_.fetch_add(num_jobs, std::memory_order_release);
        
        for (const auto& task_func : task_funcs) {
            Job new_job(task_func, type, new_handle);
            unsigned int queue_index = submit_thread_index_.fetch_add(1) % num_threads_;
            push_to_queue(queue_index, std::move(new_job));
        }

        worker_cv_.notify_all();
        return new_handle;
    }

    /**
     * @brief Stalls the calling thread until the specified handle is complete.
     * While waiting, this thread will participate in executing other jobs from the system.
     * @param handle The handle to wait for.
     */
    void wait_for(const JobHandle& handle) {
        // While the handle we are waiting for is not complete...
        while (!handle.is_complete()) {
            // ...try to execute another job from anywhere in the system.
            if (!try_execute_a_job(-1)) { // -1 indicates this is not a worker thread
                // If there was no work to steal, yield the thread to prevent a busy-wait spin.
                std::this_thread::yield();
            }
        }
    }

    /**
     * @brief Gets a consistent snapshot of the current job counts for diagnostics.
     * @return A Snapshot struct containing job counts.
     */
    Snapshot get_diagnostics_snapshot() const {
        Snapshot snapshot;
        snapshot.total_pending = total_pending_jobs_count_.load(std::memory_order_relaxed);
        for (const auto& pair : job_type_counts_) {
            snapshot.type_counts[pair.first] = pair.second.load(std::memory_order_relaxed);
        }
        return snapshot;
    }
    std::vector<ThreadDiagnostics> get_thread_diagnostics() const {
        std::vector<ThreadDiagnostics> diagnostics;
        diagnostics.resize(num_threads_);

        // First, lock dedication map to safely read dedication statuses
        {
            std::lock_guard<std::mutex> lock(dedication_mutex_);
            // Initialize all threads as General by default
            for (unsigned int i = 0; i < num_threads_; ++i) {
                diagnostics[i].thread_id = i;
                diagnostics[i].dedication_type = "General"; // Default status
            }
            // Override with specific dedications
            for (const auto& pair : thread_dedications_) {
                JobType type = pair.first;
                unsigned int thread_id = pair.second;
                if (thread_id < num_threads_) {
                    diagnostics[thread_id].dedication_type = to_string(type);
                }
            }
        }

        // Now, iterate through each thread's queue to get job counts
        for (unsigned int i = 0; i < num_threads_; ++i) {
            // Lock the specific queue we are about to inspect
            std::lock_guard<std::mutex> lock(per_queue_mutexes_[i]);
            
            const auto& queue = per_thread_queues_[i];
            diagnostics[i].job_count = queue.size();
            
            // Count the types of jobs in this thread's queue
            for (const auto& job : queue) {
                diagnostics[i].type_counts[job.type]++;
            }
        }

        return diagnostics;
    }

    /**
     * @brief Signals all worker threads to stop and joins them.
     * This ensures a clean shutdown.
     */
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
    // --- START: Integrated Queue Logic and Helpers ---

    void push_to_queue(unsigned int queue_index, Job item) {
        std::lock_guard<std::mutex> lock(per_queue_mutexes_[queue_index]);
        per_thread_queues_[queue_index].push_back(std::move(item));
    }

    bool try_pop_from_queue(unsigned int queue_index, Job& item) {
        std::lock_guard<std::mutex> lock(per_queue_mutexes_[queue_index]);
        auto& queue = per_thread_queues_[queue_index];
        if (queue.empty()) {
            return false;
        }
        item = std::move(queue.back());
        queue.pop_back();
        return true;
    }

    bool try_steal_from_queue(unsigned int queue_index, Job& item) {
        std::lock_guard<std::mutex> lock(per_queue_mutexes_[queue_index]);
        auto& queue = per_thread_queues_[queue_index];
        if (queue.empty()) {
            return false;
        }
        item = std::move(queue.front());
        queue.pop_front();
        return true;
    }
    
    void pop_all_from_queue(unsigned int queue_index, std::vector<Job>& items) {
        std::lock_guard<std::mutex> lock(per_queue_mutexes_[queue_index]);
        auto& queue = per_thread_queues_[queue_index];
        if (queue.empty()) {
            return;
        }
        items.reserve(items.size() + queue.size());
        std::move(queue.begin(), queue.end(), std::back_inserter(items));
        queue.clear();
    }
    
    // --- END: Integrated Queue Logic and Helpers ---

    /**
     * @brief The main loop for each worker thread.
     * A thread will first try to execute a job from its own queue. If its queue is empty,
     * it will attempt to steal work from other threads. If no work is found, it will sleep.
     */
    void worker_thread_loop(unsigned int thread_id, std::atomic<bool>& stop_flag, std::mutex& mtx, std::condition_variable& cv) {
        while (!stop_flag.load(std::memory_order_acquire)) {
            if (try_execute_a_job(thread_id)) {
                // If we successfully found and ran a job, immediately loop again
                // to look for more work without waiting.
                continue;
            }

            // If no work was found, go to sleep and wait for a notification.
            std::unique_lock<std::mutex> lock(worker_mutex_);
            worker_cv_.wait(lock, [&]() {
                // Wake up if stop is signaled OR if there might be work to do.
                return stop_flag.load(std::memory_order_acquire) || total_pending_jobs_count_.load(std::memory_order_acquire) > 0;
            });
        }
        logger_->info("Worker thread " + std::to_string(thread_id) + " exiting.");
    }

    /**
     * @brief Attempts to find and execute one job from any queue.
     * @param thread_id The ID of the worker thread calling, or -1 for a non-worker thread.
     * @return True if a job was found and executed, false otherwise.
     */
    bool try_execute_a_job(int thread_id) {
        Job job_to_execute;
        bool job_found = false;

        // 1. If called by a worker, try to pop from its own queue first.
        if (thread_id != -1 && try_pop_from_queue(thread_id, job_to_execute)) {
            job_found = true;
        } else {
            // 2. If not a worker or own queue is empty, try to steal from others.
            for (unsigned int i = 0; i < num_threads_; ++i) {
                if (i == thread_id) continue;

                if (try_steal_from_queue(i, job_to_execute)) {
                    job_found = true;
                    break;
                }
            }
        }
        
        if (job_found) {
            execute_job(job_to_execute);
            return true;
        }

        return false;
    }

    /**
     * @brief Executes a job and decrements its associated counters.
     * @param job The job to execute.
     */
    void execute_job(Job& job) {
        job.task();
        job.handle.get_counter()->fetch_sub(1, std::memory_order_release);
            
        job_type_counts_[job.type].fetch_sub(1, std::memory_order_release);
        total_pending_jobs_count_.fetch_sub(1, std::memory_order_release);
        
        manager_cv_.notify_all();
    }

    // --- Member Variables ---

    Logger* logger_ = nullptr;
    unsigned int num_threads_;
    std::vector<std::unique_ptr<ngin::jobs::Thread>> worker_threads_;
    
    std::vector<std::deque<Job>> per_thread_queues_;
    mutable std::vector<std::mutex> per_queue_mutexes_;
    std::atomic<unsigned int> submit_thread_index_;

    // NEW: Members for thread dedication
    mutable std::mutex dedication_mutex_;
    std::map<JobType, unsigned int> thread_dedications_;
    std::vector<unsigned int> general_purpose_thread_indices_;

    std::mutex worker_mutex_;
    std::condition_variable worker_cv_;
    std::condition_variable manager_cv_;

    std::map<JobType, std::atomic<int>> job_type_counts_;
    std::atomic<int> total_pending_jobs_count_;
};

} // namespace jobs
} // namespace ngin

#endif 