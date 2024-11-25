#ifndef NODE_CONNECTION_H
#define NODE_CONNECTION_H

#include <memory>
#include <string>
#include <ngin/node/node_port.h>
#include <ngin/node/i_node_connection.h>
#include <ngin/collections/nevf.h>

class NodeConnection : INodeConnection {
public:
    // Constructor: Automatically connects the ports
    NodeConnection(std::string name, std::string type, const std::shared_ptr<NodePort>& inputPort, const std::shared_ptr<NodePort>& outputPort)
        : name_(name), type_(type), inputPort_(inputPort), outputPort_(outputPort) {
        if (inputPort_) {
            inputPort_->connect(this); // Connect input port to this connection
        }
        if (outputPort_) {
            outputPort_->connect(this); // Connect output port to this connection
        }
    }

    // Destructor: Disconnect both ports if they are not nullptr
    ~NodeConnection() {
        if (inputPort_) {
            inputPort_->disconnect(); // Disconnect the input port
        }
        if (outputPort_) {
            outputPort_->disconnect(); // Disconnect the output port
        }
    }

    // Getter for input port
    std::shared_ptr<NodePort> getInputPort() const override { return inputPort_; }

    // Getter for output port
    std::shared_ptr<NodePort> getOutputPort() const override { return outputPort_; }

    std::shared_ptr<Nevf> getData() {
        return inputPort_->getData();
    }

    void transferData() override {
        outputPort_->setData(inputPort_->getData());
        inputPort_->clearData();
    }

    std::string& getType() override {
        return type_;
    }


    void log() const override {
        std::cout << "connection: " << name_ << std::endl; // Print connection name
        if (outputPort_) {
            std::cout << "       - connection output port: " << outputPort_->getName() << " (id: " << outputPort_->getId() << ")" << std::endl; // Print output port name and ID
        }
    }

private:
    std::string name_;
    std::string type_;

    std::shared_ptr<NodePort> inputPort_;  // Input port
    std::shared_ptr<NodePort> outputPort_; // Output port
};

#endif // NODE_CONNECTION_H
