#ifndef COROUTINE_H
#define COROUTINE_H

#include <coroutine>
#include <iostream>
#include <chrono>
#include <thread>

struct AwaitableTimer {
    std::chrono::milliseconds delay;

    bool await_ready() const noexcept {
        return delay.count() <= 0; // No waiting if delay is 0 or negative
    }

    void await_suspend(std::coroutine_handle<> handle) const {
        std::thread([handle, this] {
            std::this_thread::sleep_for(delay); // Simulate waiting
            handle.resume(); // Resume the coroutine after the delay
        }).detach(); // Detach the thread to run independently
    }

    void await_resume() const noexcept {
        // Nothing to do here
    }
};

struct Coroutine {
    struct promise_type {
        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    std::coroutine_handle<promise_type> coro;
    Coroutine(std::coroutine_handle<promise_type> h) : coro(h) {}
    ~Coroutine() {
        if (coro) coro.destroy();
    }
};

#endif // COROUTINE_H