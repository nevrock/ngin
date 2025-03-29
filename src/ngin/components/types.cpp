
#include <ngin/scene/object.h>  
#include <ngin/components/light.h>
#include <ngin/components/light_point.h>
#include <ngin/components/camera.h>
#include <ngin/components/mesh.h>
#include <ngin/components/quad.h>
#include <ngin/components/gui_quad.h>
#include <ngin/components/sphere.h>
#include <ngin/components/spheres.h>
#include <ngin/components/skybox.h>

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

bool quad_registered = []() {
    Object::registerComponent("quad", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Quad>(name, lex, parent);
    });
    return true;
}();

bool sphere_registered = []() {
    Object::registerComponent("sphere", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Sphere>(name, lex, parent);
    });
    return true;
}();

bool spheres_registered = []() {
    Object::registerComponent("spheres", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Spheres>(name, lex, parent);
    });
    return true;
}();

bool skybox_registered = []() {
    Object::registerComponent("skybox", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<Skybox>(name, lex, parent);
    });
    return true;
}();

bool gui_quad_registered = []() {
    Object::registerComponent("gui_quad", [](const std::string name, const Lex& lex, IObject* parent) {
        return std::make_unique<GuiQuad>(name, lex, parent);
    });
    return true;
}();