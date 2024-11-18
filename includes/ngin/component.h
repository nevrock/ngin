#ifndef COMPONENT_H
#define COMPONENT_H

#include <ngin/dict.h> // Include the JSON library

#include <ngin/i_object.h>
#include <ngin/shader.h>
#include <ngin/point.h>
#include <ngin/log.h>

class Object;

class Component {
public:
    Component(IObject* parentObj) : parent(parentObj) {}
    virtual ~Component() {}

    virtual void loadFromDict(const Dict& d) = 0;

    virtual void update() {}
    virtual void updatePhysics(float dt) {}
    virtual void updateAnimation(float dt) {}

    virtual void updatePreRender(const unsigned int index, Shader& shader) {}
    virtual void updateRender(const unsigned int index, Shader& shader) {}
    virtual void launch() {}

    const Point& getPoint() const {
        if (parent) {
            return parent->getPoint();
        }
        static const Point defaultConstPoint; // Fixed const instance to return by reference
        return defaultConstPoint; // Return a default const Point if parent is null
    }
    Point& getPoint() {
        if (parent) {
            return parent->getPoint();
        }
        static Point defaultPoint; // Fixed instance to return by reference
        return defaultPoint; // Return a default Point if parent is null
    }     
    IObject* getParent() {
        return parent;
    }

protected:
    IObject* parent; 
};

#endif // COMPONENT_H
