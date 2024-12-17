#ifndef TIME_H
#define TIME_H

#include <ngin/collections/nevf.h>
#include <memory>
#include <functional>
#include <atomic>

class Time {
public:
    static void init() {
        Log::console("!!!   time started   !!!");
    }
    static void setData(std::shared_ptr<Nevf> nevf) {
        data_ = nevf;
    }
    static std::shared_ptr<Nevf> getData() {
        return data_;
    }

private:
    // Constructor, Copy constructor, and Assignment operator are private to prevent multiple instances
    Time() {}
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;

    static std::shared_ptr<Nevf> data_;
};

#endif // TIME_H