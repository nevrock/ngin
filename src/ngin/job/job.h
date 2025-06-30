#ifndef JOB_H
#define JOB_H 

#include <functional> // For std::function
#include <memory>     // For std::shared_ptr if JobHandle is shared_ptr based
#include <atomic>     // For atomic operations within JobHandle

#include <ngin/debug/logger.h> // Assuming this path is correct in your project
#include <ngin/job/handle.h>   // Include JobHandle definition

enum class JobType {
    None,
    Physics,
    Animation,
    AI,
    TransformSync, // For syncing physics/animation results to scene graph
    RenderCommandGeneration,
    AssetLoading,
    Other
};

// --- Job Struct ---
// Represents a single unit of work to be executed by a worker thread.
// It contains the task, its type, and the handle it contributes to.
struct Job {
    std::function<void()> task; // The actual work to perform
    JobType type;               // The category of this job
    JobHandle handle;           // The handle this job belongs to (for decrementing its counter)

    // Default constructor for cases where it's default-constructed (e.g., in a queue)
    Job() : type(JobType::None) {}

    // Constructor to initialize a job
    Job(std::function<void()> fn, JobType t, JobHandle h)
        : task(std::move(fn)), type(t), handle(std::move(h)) {}
};

#endif