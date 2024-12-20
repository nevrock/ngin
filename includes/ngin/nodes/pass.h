#ifndef PASS_H
#define PASS_H

#include <string>
#include <tuple>

#include <ngin/node/node.h>

// Object class inheriting from Node
class Pass : public Node {
public:
    Pass(const std::string& name, Nevf& data) : Node(name, data) {
    }

    // Override the execute method
    void update(std::string& pass) override {
        Node::update(pass); // retrieve inputs
    }

    void setup() override {
        Node::setup();
        pass_ = data_.getC<std::string>("pass", ""); // Get the "pass" value from data_
    }

    std::string getPass() const { return pass_; } // Public getter method

private:
    std::string pass_; // Private string variable to store the pass
};

#endif // PASS_H