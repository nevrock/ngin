#ifndef GAME_H
#define GAME_H

#include <ngin/debug/logger.h>

#include <ngin/game/scene/scene.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include <map>

class Game {
public:
    // Constructor to initialize assets
    Game() : logger_(new Logger("Game")) {
        logger_->info("Initializing game");
    }
    ~Game() {
        delete logger_;
        for (auto& scene : scenes_) {
            delete scene.second;
        }
        scenes_.clear();
    }

    void setup() {
        logger_->info("Game setup");

        // Always setup an main scene and gui scene
        Scene* main_scene = new Scene("main");
        main_scene->build("origin_main");
        scenes_["main"] = main_scene;

        Scene* gui_scene = new Scene("gui");
        gui_scene->build("origin_gui");
        scenes_["gui"] = gui_scene;
    }
    void update() {

    }
    void upate_render() {
        
    }
    void update_physics() {

    }
    void cleanup() {
        logger_->info("Game cleanup");
    }

private:
    Logger* logger_;
    std::map<std::string, Scene*> scenes_;
};

#endif // GAME_H