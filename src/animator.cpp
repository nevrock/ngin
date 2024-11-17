#include <snorri/animator.h>

Animator::Animator(Object* parent) 
    : Component(parent) {
}

bool animator_registered = []() {
    Object::registerComponent("animator", [](Object* parent) {
        return std::make_shared<Animator>(parent);
    });
    return true;
}();
