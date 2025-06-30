#ifndef GUI_IMAGE_H
#define GUI_IMAGE_H

#include <vector>
#include <glm/glm.hpp>

#include <ngin/game/scene/module.h>

class GuiImage : public Module {
public:
    GuiImage(const std::string name, Object& parent, Atlas& data)
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

#endif // GUI_IMAGE_H
