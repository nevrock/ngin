#ifndef NGIN_H
#define NGIN_H

#include <memory>
#include <functional>
#include <atomic>

#include <ngin/log.h>
#include <ngin/lex.h>

#include <ngin/game.h>
#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <ngin/lighter.h>

class Ngin {
public:

    static void init() {
        Game::init();
        Resources::init();
        Drawer::init();
        Lighter::init();
    }
    static void start() {
        Game::start();
        Drawer::start();
    }
    static void end() {
        Resources::terminate();
        Game::terminate();
    }
    static void updateRender() {
        //Log::console("update render");
    }
    static void updatePhysics() {
        //Log::console("update physics");
    }
    
    static void updateLogic() {
        //Log::console("update logic");
        Game::updateLogic();
    }
    static void updateTransform() {
        //Log::console("update transform");
        Game::updateTransform();
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

private:
    static inline std::shared_ptr<Lex> env_;

};

#endif // NGIN_H