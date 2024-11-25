#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>
#include <tuple>

#include <ngin/node/node.h>

// Object class inheriting from Node
class Transform : public Node {
public:
    Transform(const std::string& name, Nevf& data) : Node(name, data) {}

    // Override the execute method
    void execute(std::string& pass) override {
        // Example: Logic specific to Object
        // This can be customized based on requirements
    }

    void setup() override {

    }
private:

};

#endif // TRANSFORM_Hr