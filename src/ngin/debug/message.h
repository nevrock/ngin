#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <chrono>
#include <string>
// No need for <vector>, <algorithm>, <iostream>, <iomanip>, <sstream> here,
// as LogMessage just defines the struct, not its usage.

namespace ngin {
namespace debug {

// 1. LogMessage Struct
struct LogMessage {
    std::chrono::system_clock::time_point timestamp;
    std::string type;
    std::string message;

    // Default constructor for use with ParallelQueue
    LogMessage() = default; 

    LogMessage(std::chrono::system_clock::time_point ts, std::string type_str, std::string msg)
        : timestamp(std::move(ts)), type(std::move(type_str)), message(std::move(msg)) {}

    // Comparison operator for sorting by timestamp
    // Sorting by time_point already includes sub-second precision.
    bool operator<(const LogMessage& other) const {
        return timestamp < other.timestamp;
    }
};

}
}

#endif // LOG_MESSAGE_H