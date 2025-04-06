#ifndef ARMATURE_H
#define ARMATURE_H

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>

#include <ngin/log.h>

#include <ngin/resources.h>    

class Armature : public IComponent {
public:
    Armature(const std::string name, const Lex& lex, IObject* parent)
        : IComponent(name, lex, parent) {

    }
    ~Armature() {

    }

    void init() override {

    }
    void launch() override {
        // Implementation of launch method
    }
    void update() override {
        // Implementation of update method
    }
    void updateLate() override {
        // Implementation of updateLate method
    }

private:

};

#endif // ARMATURE_H
