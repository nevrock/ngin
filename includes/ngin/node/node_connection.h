#ifndef NODE_CONNECTION_H
#define NODE_CONNECTION_H

#include <memory>
#include <string>
#include <ngin/node/node_port.h>
#include <ngin/node/i_node_connection.h>
#include <ngin/data/i_data.h>
#include <ngin/collections/nevf.h>

#include <ngin/log.h>   

class NodeConnection : public INodeConnection, public std::enable_shared_from_this<NodeConnection> {
public:
    // Constructor: Automatically connects the ports
    NodeConnection(std::string name, std::string type, const std::shared_ptr<NodePort>& inputPort, const std::shared_ptr<NodePort>& outputPort)
        : name_(name), type_(type), inputPort_(inputPort), outputPort_(outputPort) {

        //std::cout << "new node connection! " << inputPort->getName() << ", " << inputPort->getId() << " --> " << outputPort->getName() << ", " << outputPort->getId() << std::endl;
    }

    // Destructor: Disconnect both ports if they are not nullptr
    ~NodeConnection() {
        if (inputPort_.lock()) {
            inputPort_.lock()->disconnect(); // Disconnect the input port
        }
        if (outputPort_.lock()) {
            outputPort_.lock()->disconnect(); // Disconnect the output port
        }
    }

    // Getter for input port
    std::weak_ptr<NodePort> getInputPort() const override { return inputPort_; }

    // Getter for output port
    std::weak_ptr<NodePort> getOutputPort() const override { return outputPort_; }

    template <typename T>
    std::shared_ptr<T> getData() {
        if (auto spt = inputPort_.lock()) { 
            return spt->getData<T>();        
        } else {
            return nullptr; 
        }
    }
    
    void transferData() override {
        if (auto input = inputPort_.lock(); input) {
            if (auto output = outputPort_.lock()) {
                // Get the raw IData pointer
                std::shared_ptr<IData> dataToTransfer = input->getRawData(); 
                output->setRawData(dataToTransfer); 

                //Log::console("Transferring data from " + input->getName() + " to " + output->getName());
                //input->clearData();
            } else { 
                // Handle the case where output port is gone
            }
        } else {
            // Handle the case where input port is gone 
        }
    }

    std::string getType() override {
        return type_;
    }


    void log() const override {
        //std::cout << "connection: " << name_ << std::endl; // Print connection name
        if (outputPort_.lock()) {
            //std::cout << "       - connection output port: " << outputPort_.lock()->getName() << " (id: " << outputPort_.lock()->getId() << ")" << std::endl; // Print output port name and ID
        }
    }

private:
    std::string name_;
    std::string type_;

    std::weak_ptr<NodePort> inputPort_;  // Input port
    std::weak_ptr<NodePort> outputPort_; // Output port
};

#endif // NODE_CONNECTION_H
