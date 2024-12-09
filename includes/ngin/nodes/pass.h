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
    void execute(std::string& pass) override {
        // Example: Logic specific to Object
        // This can be customized based on requirements

        Node::execute(pass); // retrieve inputs

        std::vector<std::shared_ptr<NodePort>> outputPorts = getOutputPortsByType(pass);
        std::shared_ptr<NodePort> inputPort = getInputPortByType(pass);
        if (!inputPort) {
            return;
        }
        for (const auto& port : outputPorts) {
            // Do something with each port
            port->setRawData(inputPort->getRawData());
        }
    }

    void setup() override {
        Node::setup();
        pass_ = data_.getC<std::string>("pass", ""); // Get the "pass" value from data_
        order_ = data_.getC<int>("order", 0);
    }

    std::string getPass() const { return pass_; } // Public getter method
    int getOrder() const { return order_; }

private:
    std::string pass_; // Private string variable to store the pass
    int order_;
};

#endif // PASS_H