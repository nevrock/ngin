#include <nev/surface_sdf.h>

void SurfaceSdf::loadFromDict(const Dict& d) {    
}

bool surface_sdf_registered = []() {
    try {
        Object::registerComponent("surface_sdf", [](Object* parent) {
            return std::make_shared<SurfaceSdf>(parent);
        });
    } catch (const std::exception& e) {
        Log::console(std::string("Failed to register body: ") + e.what());
        return false;
    }
    return true;
}();
