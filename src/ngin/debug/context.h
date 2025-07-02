#ifndef DEBUG_CONTEXT_H
#define DEBUG_CONTEXT_H

#include <ngin/job/collections/queue.h>
#include <ngin/debug/message.h>

#include <chrono>
#include <vector>
#include <string>
#include <algorithm> // For std::sort
#include <iostream>
#include <iomanip>   // For std::put_time
#include <sstream>   // For std::stringstream

#include <ngin/debug/printer.h>

namespace ngin {
namespace debug {

// 2. DebugContext Class
class DebugContext : public Printer {
public:
    // Constructor (no longer private, as it's not a singleton)
    DebugContext(ngin::jobs::ParallelQueue<LogMessage>& log_queue) : Printer(), log_queue_(log_queue) {}

    // Adds a log message to the queue with the current timestamp and type
    void info(const std::string& message, std::string type_str = "", unsigned int indent = 0) override {
        auto now = std::chrono::system_clock::now();
        log_queue_.push(LogMessage(now, type_str, message));
    }

private:
    ngin::jobs::ParallelQueue<LogMessage>& log_queue_;
};

}
}

#endif // DEBUG_CONTEXT_H