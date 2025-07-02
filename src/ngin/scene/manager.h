#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <memory>

#include <ngin/debug/logger.h>

#include <ngin/scene/object/manager.h>

namespace ngin {
namespace scene {

class SceneManager {
public:
    SceneManager(std::string name) : name_(name), object_mgr_() {
    }
    ~SceneManager() {
    }

    void build() {
        // Atlas* origin_data = NGIN::assets().get<Atlas>("object", origin_asset_name);
    }

private:
    std::string name_;

    ObjectManager object_mgr_;
};

}
}

#endif // SCENE_MANAGER_H
