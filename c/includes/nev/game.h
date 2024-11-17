#ifndef GAME_H
#define GAME_H

#include <nev/event.h>
#include <memory>
#include <functional>
#include <atomic>

class Game {
public:
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
};

#endif // GAME_H
