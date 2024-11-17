#ifndef COROUTINE_MANAGER_H
#define COROUTINE_MANAGER_H

#include <nev/coroutine.h>
#include <thread>
#include <iostream>
#include <atomic>
#include <condition_variable>
#include <mutex>

class CoroutineManager {
public:
    CoroutineManager() : coro(nullptr), isRunning(false) {}
    ~CoroutineManager() {
        stop();
        if (worker.joinable()) {
            worker.join();
        }
        if (coro) {
            coro.destroy();
        }
    }

    void start() {
        isRunning = true;
        worker = std::thread(&CoroutineManager::run, this);
    }

    void stop() {
        std::unique_lock<std::mutex> lock(mutex);
        isRunning = false;
        cv.notify_one();
    }

private:
    std::coroutine_handle<> coro;
    std::thread worker;
    std::atomic<bool> isRunning;
    std::condition_variable cv;
    std::mutex mutex;

    static void run(CoroutineManager* manager) {
        try {
            auto coroutine = exampleCoroutine();
            manager->coro = coroutine.coro;

            std::unique_lock<std::mutex> lock(manager->mutex);
            while (!manager->coro.done() && manager->isRunning) {
                manager->cv.wait_for(lock, std::chrono::milliseconds(100), [manager] {
                    return manager->coro.done() || !manager->isRunning;
                });
                std::cout << "Coroutine is still running..." << std::endl;
            }
            std::cout << "Coroutine completed successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception in coroutine: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in coroutine." << std::endl;
        }

        manager->isRunning = false;
    }

    static Coroutine exampleCoroutine() {
        std::cout << "Coroutine started. Waiting for 2 seconds..." << std::endl;
        co_await AwaitableTimer{std::chrono::milliseconds(2000)};
        std::cout << "Coroutine resumed and completed." << std::endl;
        co_return;
    }
};

#endif // COROUTINE_MANAGER_H
