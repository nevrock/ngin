#ifndef DEBUG_BUCKET_H
#define DEBUG_BUCKET_H

#include <ngin/job/collections/queue.h>
#include <ngin/debug/message.h>
#include <ngin/debug/context.h>

#include <chrono>
#include <vector>
#include <string>
#include <algorithm> // For std::sort
#include <iostream>
#include <iomanip>   // For std::put_time and std::setfill, std::setw
#include <sstream>   // For std::stringstream

namespace ngin {
namespace debug {
// 2. DebugBucket Class
class DebugBucket {
public:
    // Constructor (no longer private, as it's not a singleton)
    DebugBucket() = default; 

    // Adds a log message to the queue with the current timestamp and type
    void info(const std::string& type_str, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        log_queue_.push(LogMessage(now, type_str, message));
    }

    // Pops all messages, sorts them by timestamp, and prints them with the specified format
    void show() {
        std::vector<LogMessage> messages;
        log_queue_.pop_all(messages);

        // Sort messages by timestamp
        // std::chrono::system_clock::time_point inherently supports high precision for sorting.
        std::sort(messages.begin(), messages.end());

        // Print sorted messages
        for (const auto& log_msg : messages) {
            // Get milliseconds from the time_point
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log_msg.timestamp.time_since_epoch()) % 1000;
            
            // Convert time_point to std::time_t for formatting
            std::time_t time = std::chrono::system_clock::to_time_t(log_msg.timestamp);
            std::tm tm_snapshot;

        #ifdef _WIN32
            localtime_s(&tm_snapshot, &time);
        #else
            localtime_r(&time, &tm_snapshot);
        #endif

            std::stringstream ss;
            // Format to HH:MM:SS.ms (removed date)
            ss << std::put_time(&tm_snapshot, "%H:%M:%S") << '.'
               << std::setfill('0') << std::setw(3) << ms.count();
            
            // Format output similar to ngin::debug::Logger class
            std::cout << ss.str() << " [INFO]::(parallel)::[\e[3m" << log_msg.type << "\e[0m]"
                      << " " << log_msg.message << std::endl;
        }
    }

    DebugContext& get_context() {
        return context_;
    }

private:
    ngin::jobs::ParallelQueue<LogMessage> log_queue_;
    DebugContext context_ = DebugContext(log_queue_);
};

}
}

#endif // DEBUG_BUCKET_H