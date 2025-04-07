
#include <ngin/scene/object.h>  
#include <ngin/components/light.h>
#include <ngin/components/light_point.h>
#include <ngin/components/camera.h>
#include <ngin/components/mesh.h>
#include <ngin/components/armature.h>

#include <ngin/components/gui_quad.h>
#include <ngin/components/gui_font.h>
#include <ngin/components/gui_group.h>

bool light_registered = []() {
    Object::registerComponent("light", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Light>(name, lex, parent);
    });
    return true;
}();

bool light_point_registered = []() {
    Object::registerComponent("light_point", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<LightPoint>(name, lex, parent);
    });
    return true;
}();

bool camera_registered = []() {
    Object::registerComponent("camera", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Camera>(name, lex, parent);
    });
    return true;
}();

bool mesh_registered = []() {
    Object::registerComponent("mesh", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Mesh>(name, lex, parent);
    });
    return true;
}();

bool armature_registered = []() {
    Object::registerComponent("armature", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Armature>(name, lex, parent);
    });
    return true;
}();    

bool gui_quad_registered = []() {
    Object::registerComponent("gui_quad", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<GuiQuad>(name, lex, parent);
    });
    return true;
}();

bool gui_font_registered = []() {
    Object::registerComponent("gui_font", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<GuiFont>(name, lex, parent);
    });
    return true;
}();

bool gui_group_registered = []() {
    Object::registerComponent("gui_group", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<GuiGroup>(name, lex, parent);
    });
    return true;
}();