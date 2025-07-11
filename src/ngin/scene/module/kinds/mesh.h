#ifndef MESH_MODULE_H
#define MESH_MODULE_H

#include <string>
#include <vector>
#include <algorithm>

#include <ngin/scene/module/module.h>
#include <ngin/atlas/atlas.h>

namespace ngin {
namespace scene {

class MeshModule : public Module {
public:
    MeshModule (const unsigned int id, const std::string& name) : Module(id, name) {

    } // Initialize parent_ here

    void from_atlas(Atlas* data) override {
    }

private:
};
}
}

#endif // MESH_MODULE_H