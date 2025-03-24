#ifndef GAME_H
#define GAME_H

#include <memory>
#include <functional>
#include <atomic>

#include <ngin/event.h>
#include <ngin/lex.h>
#include <ngin/log.h>
#include <ngin/resources.h>
#include <ngin/scene/scene.h>

class Game {
public:
    static void init() {
        std::cout << "" << std::endl;
        std::cout << "-------------------------------" << std::endl;
        std::cout << "### PHASE - STATIC ###" << std::endl;
        std::cout << "-------------------------------" << std::endl;

        game_ = std::make_shared<Lex>(Resources::loadLexicon("game"));
        // game_->print();
        env_ = std::make_shared<Lex>(game_->getC<Lex>("env", Lex()));

        running_ = true;
    }
    static void start() {
        scene_ = std::make_unique<Scene>("scene");
        scene_->build();
        scene_->launch();
    }
    static void updateLogic() {
        //Log::console("!!!   game update logic   !!!");
        scene_->updateLogic();
    }
    static void updateTransform() {
        //Log::console("!!!   game update logic   !!!");
        scene_->updateTransform();
    }
    static void terminate() {
    }
    static void setEnv(std::shared_ptr<Lex> nevf) {
        env_ = nevf;
    }
    static std::shared_ptr<Lex> getEnv() {
        return env_;
    }
    template <typename T>
    static T envget(const std::string& key) { 
        return env_->getC<T>(key, T());
    }
    template <typename T>
    static void envset(const std::string& key, T value) {
        env_->set(key, value);
    }
    static std::string& getState() {
        return state_;
    }
    static void setState(const char* s) {
        state_ = std::string(s);
        eventState_.trigger();
    }
    static void addStateListener(std::function<void(const Lex&)> listener) {
        eventState_.addListener(listener);
    }
    static void setRunning(bool running) {
        running_ = running;
    }
    static bool isRunning() {
        return running_;
    }

private:
    // Constructor, Copy constructor, and Assignment operator are private to prevent multiple instances
    Game() {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    static std::string state_;
    static Event eventState_;

    static std::shared_ptr<Lex> env_;
    static std::shared_ptr<Lex> game_;
    static inline std::unique_ptr<Scene> scene_;

    static bool running_;
};

#endif // GAME_H