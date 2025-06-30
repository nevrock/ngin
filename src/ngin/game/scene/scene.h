#ifndef SCENE_H
#define SCENE_H

#include <memory>

#include <ngin/game/scene/game_object.h>
#include <ngin/assets/assets.h>
#include <ngin/debug/logger.h>
#include <ngin/ngin.h>


class Scene {
public:
    Scene(std::string name) : name_(name) {
        logger_ = new Logger("Scene");
    }
    ~Scene() {
        delete origin_;
        delete logger_;
    }

    void build(std::string origin_asset_name) {
        Atlas* origin_data = NGIN::assets().get<Atlas>("object", origin_asset_name);
        if (origin_data) {
            origin_ = new GameObject("origin", *origin_data);
            origin_->build();
        } else {
            logger_->warn("Origin asset not found: " + origin_asset_name);
        }
    }

private:
    Object* origin_;
    Logger* logger_;

    std::string name_;
};


#endif // SCENE_H
