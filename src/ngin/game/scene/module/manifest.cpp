
#include <ngin/game/scene/object.h>  

#include <ngin/game/scene/module/mesh.h>

bool mesh_registered = []() {
    Object::register_module("mesh", [](const std::string name, Object& parent, Atlas& data) {
        return std::make_unique<Mesh>(name, parent, data);
    });
    return true;
}();
