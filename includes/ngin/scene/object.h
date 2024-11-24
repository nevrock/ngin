#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <tuple>

#include <ngin/node/node.h>
#include <ngin/collections/nevf.h>

// Object class inheriting from Node
class Object : public Node {
public:
    Object(const std::string& name, Nevf& data) : Node(name, data) {
        setupFromData();
    }

    // Set the position of the object
    void setPosition(float x, float y, float z) {
        position_ = {x, y, z};
    }

    // Get the position of the object
    std::tuple<float, float, float> getPosition() const {
        return position_;
    }

    // Override the execute method
    void execute() override {
        // Example: Logic specific to Object
        // This can be customized based on requirements
    }

private:
    std::tuple<float, float, float> position_ = {0.0f, 0.0f, 0.0f};
};

#endif // OBJECT_H
