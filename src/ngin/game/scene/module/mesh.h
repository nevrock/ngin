#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

#include <ngin/game/scene/module.h>

class Mesh : public Module {
public:
    Mesh(const std::string name, Object& parent, Atlas& data)
        : Module(name, parent, data) {
    }

    void init() override {
    }
    void launch() override {

    }

    void update() override {
    }
    void update_late() override {
    }


private:
};

#endif // MESH_H
