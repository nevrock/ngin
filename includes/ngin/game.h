#ifndef GAME_H
#define GAME_H

#include <ngin/event.h>
#include <ngin/collections/nevf.h>
#include <memory>
#include <functional>
#include <atomic>

class Game {
public:
    static void init() {
        std::cout << "!!!   game started   !!!" << std::endl;
    }
    static void setEnv(std::shared_ptr<Nevf> nevf) {
        env_ = nevf;
    }
    static std::shared_ptr<Nevf> getEnv() {
        return env_;
    }
    static std::string& getState() {
        return state_;
    }
    static void setState(const char* s) {
        state_ = std::string(s);
        eventState_.trigger();
    }
    static void addStateListener(std::function<void()> listener) {
        eventState_.addListener(listener);
    }

private:
    // Constructor, Copy constructor, and Assignment operator are private to prevent multiple instances
    Game() {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    static std::string state_;
    static Event eventState_;

    static std::shared_ptr<Nevf> env_;
};

#endif // GAME_H