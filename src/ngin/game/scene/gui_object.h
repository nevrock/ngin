#ifndef GUI_OBJECT_H
#define GUI_OBJECT_H

#include <map>
#include <memory>

#include <ngin/game/scene/object.h>

#include <ngin/debug/logger.h>
#include <ngin/atlas/atlas.h>
#include <ngin/game/scene/module.h>

class GuiObject : public Object {
public:
    GuiObject() : Object() {
        logger_ = new Logger("GuiObject : Object");
    }
    GuiObject(std::string name, Atlas& data) : Object(name, data) {
        logger_ = new Logger("GuiObject : Object::" + name);
    }
    ~GuiObject() {
        for (auto& child : children_) {
            delete child.second;
        }
        for (auto& module : modules_) {
            delete module;
        }
    }

    void build() override {}

    // Initialization and launch methods
    void init() override {}
    void launch() override {}

    // Update methods in order
    void update_logic() override {} // Perform any movement related logic here
    void update_transform(glm::mat4 parentModel = glm::mat4(1.0f)) override {}
    void update_late() override {} // Transform is fixed at this stage

private:


};

#endif // GUI_OBJECT_H
