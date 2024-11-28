#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory> // For std::shared_ptr
#include <functional> // For std::function
#include <algorithm> // For std::find_if

#include <ngin/node/node_connection.h>
#include <ngin/node/i_node.h>
#include <ngin/collections/nevf.h>

class Node : public INode, public std::enable_shared_from_this<Node> {
public:
    unsigned int depth;

    Node(const std::string& name, Nevf& dictionary)
        : name_(name), data_(dictionary) {
    }

    virtual ~Node() {}

    std::string getName() const override {
        return name_;
    }

    std::vector<std::shared_ptr<INode>> getParentNodes(std::string type) override {
        std::vector<std::shared_ptr<INode>> nodesOut;
        for (const auto& inputPort : inputPorts_) {
            if (inputPort->getType() != type) {
                continue;
            }
            if (inputPort->isConnected()) {
               // std::cout << "node " << getName() << ", port: " << inputPort->getId() << " is connected!" << std::endl;

                auto connection = inputPort->getConnection().lock(); // Lock the weak_ptr
                if (connection) { // Check if the connection is still valid
                    auto outputPort = connection->getInputPort().lock();
                    auto parentNodeWeak = outputPort->getNode(); // Get weak_ptr to INode
                    auto parentNodeShared = parentNodeWeak.lock(); // Lock the weak_ptr

                    if (parentNodeShared && std::find(nodesOut.begin(), nodesOut.end(), parentNodeShared) == nodesOut.end()) {
                        nodesOut.push_back(parentNodeShared);
                    } else {
                       // std::cout << "node " << getName() << ", port: " << inputPort->getId() << " could not pass list check!" << std::endl;
                    }
                } else {
                   // std::cout << "input port is connected but has invalid connection weak ptr" << std::endl;
                }
            } else {
               // std::cout << "node " << getName() << ", port: " << inputPort->getId() << " is NOT connected!" << std::endl;
            }
        }
        //std::cout << "node " << getName() << ", has parent size: " << nodesOut.size() << std::endl;
        return nodesOut;
    }

    std::shared_ptr<NodePort> createInputPort(const std::string& name, unsigned int id, const std::string& type) {
        // Check if a port with the same ID already exists using getInputPortById
        auto existingPort = getInputPortById(id);
        if (existingPort) {
            return existingPort; // Return the existing port if found
        }

        // Create a new port if one with the specified ID doesn't exist
        auto port = std::make_shared<NodePort>(name, id, type, weak_from_this()); 
        inputPorts_.push_back(port);
        return port;
    }

    std::shared_ptr<NodePort> createOutputPort(const std::string& name, unsigned int id, const std::string& type) {
        // Check if a port with the same ID already exists using getOutputPortById
        auto existingPort = getOutputPortById(id); // Assuming similar getOutputPortById exists, like getInputPortById
        if (existingPort) {
            return existingPort; // Return the existing port if found
        }

        // Create a new port if one with the specified ID doesn't exist
        auto port = std::make_shared<NodePort>(name, id, type, weak_from_this()); 
        outputPorts_.push_back(port);
        return port;
    }

    unsigned int getDepth() const {
        return depth;
    }

    void setDepth(unsigned int depthIn) {
        depth = depthIn;
    }

    void addInputPort(const std::shared_ptr<NodePort>& port) {
        inputPorts_.push_back(port);
    }

    void addOutputPort(const std::shared_ptr<NodePort>& port) {
        outputPorts_.push_back(port);
    }

    std::vector<std::shared_ptr<NodePort>>& getInputPorts() {
        return inputPorts_;
    }

    std::vector<std::shared_ptr<NodePort>>& getOutputPorts() {
        return outputPorts_;
    }

    std::shared_ptr<NodePort> getInputPortById(unsigned int id) {
        auto it = std::find_if(inputPorts_.begin(), inputPorts_.end(),
                               [id](const std::shared_ptr<NodePort>& port) { return port->getId() == id; });
        return (it != inputPorts_.end()) ? *it : nullptr;
    }
    std::shared_ptr<NodePort> getInputPortByType(std::string type) {
        auto it = std::find_if(inputPorts_.begin(), inputPorts_.end(),
                               [type](const std::shared_ptr<NodePort>& port) { return port->getType() == type; });
        return (it != inputPorts_.end()) ? *it : nullptr;
    }
    std::vector<std::shared_ptr<NodePort>> getOutputPortsByType(const std::string& type) const {
        std::vector<std::shared_ptr<NodePort>> result;
        for (const auto& port : outputPorts_) {
            if (port->getType() == type) {
            result.push_back(port);
            }
        }
        return result;
    }
    std::shared_ptr<NodePort> getInputPortByConnection(const std::string& type) {
        for (const auto& inputPort : inputPorts_) {
            auto connection = inputPort->getConnection().lock(); // Lock the weak_ptr
            if (connection && inputPort->isConnected() && connection->getType() == type) {
                return inputPort;
            }
        }
        return nullptr; // Return nullptr if no match is found
    }
    std::shared_ptr<IData> getInputDataByType(const std::string& type) {
        auto port = getInputPortByConnection(type);
        if (port) {
            std::shared_ptr<IData> data = port->getRawData(); // Retrieve the data from the port
            return data;
        }
        throw std::runtime_error("No connected input port with the specified type: " + type);
    }

    std::shared_ptr<NodePort> getOutputPortByConnection(const std::string& type) {
        for (const auto& outputPort : outputPorts_) {
            auto connection = outputPort->getConnection().lock(); // Lock the weak_ptr
            if (connection && outputPort->isConnected() && connection->getType() == type) {
                return outputPort;
            }
        }
        return nullptr; // Return nullptr if no match is found
    }

    void setOutputDataByType(const std::string& type, std::shared_ptr<IData> data) {
        auto port = getOutputPortByConnection(type);
        if (port) {
            port->setRawData(data); // Set the data for the port
            return;
        }
        throw std::runtime_error("No connected output port with the specified type: " + type);
    }

    std::shared_ptr<NodePort> getInputPortByName(const std::string& name) {
        auto it = std::find_if(inputPorts_.begin(), inputPorts_.end(),
                               [&name](const std::shared_ptr<NodePort>& port) { return port->getName() == name; });
        return (it != inputPorts_.end()) ? *it : nullptr;
    }

    std::shared_ptr<NodePort> getOutputPortById(unsigned int id) {
        auto it = std::find_if(outputPorts_.begin(), outputPorts_.end(),
                               [id](const std::shared_ptr<NodePort>& port) { return port->getId() == id; });
        return (it != outputPorts_.end()) ? *it : nullptr;
    }

    std::shared_ptr<NodePort> getOutputPortByName(const std::string& name) {
        auto it = std::find_if(outputPorts_.begin(), outputPorts_.end(),
                               [&name](const std::shared_ptr<NodePort>& port) { return port->getName() == name; });
        return (it != outputPorts_.end()) ? *it : nullptr;
    }

    void log() const {
        std::cout << "-------------" << std::endl;
        std::cout << "NODE: " << name_ << std::endl;
        std::cout << std::endl;

        std::cout << "  input ports: " << inputPorts_.size() << std::endl;
        for (const auto& port : inputPorts_) {
            std::cout << "    - " << port->getName() << " (id: " << port->getId() << ")";
            if (port->isConnected()) {
                auto connection = port->getConnection().lock(); // Lock the weak_ptr
                if (connection) { // Check if the connection is valid
                    std::cout << " -> ";
                    connection->log();
                }
            }
        }

        std::cout << std::endl;
        std::cout << "  output ports: " << outputPorts_.size() << std::endl;
        for (const auto& port : outputPorts_) {
            std::cout << "    - " << port->getName() << " (id: " << port->getId() << ")";
            if (port->isConnected()) {
                auto connection = port->getConnection().lock(); // Lock the weak_ptr
                if (connection) { // Check if the connection is valid
                    std::cout << " -> ";
                    connection->log();
                }
            }
        }
        std::cout << std::endl;
    }

    void execute(std::string& pass) override {
        retrieveInputData();
    }

    void setup() override { 
        std::cout << "node setup: " << getName() << std::endl;
        setupPorts();
        //std::cout << "node setup!" << std::endl; 
        }

    void launch() override { 
        }

protected:
    std::string name_;
    std::vector<std::shared_ptr<NodePort>> inputPorts_;
    std::vector<std::shared_ptr<NodePort>> outputPorts_;
    Nevf& data_;

    void retrieveInputData() {
        for (const auto& inputPort : inputPorts_) {
            if (inputPort->isConnected()) {
                auto connection = inputPort->getConnection().lock(); // Lock the weak_ptr
                if (connection) { // Check if the connection is valid
                    connection->transferData();
                }
            }
        }
        // Now all data is sitting in input ports, ready to be pulled
    }
    void setupPorts() {
        std::cout << "setting up ports!" << std::endl;
        data_.print();
        if (data_.contains("input_ports")) {
            Nevf inputData = data_.getC<Nevf>("input_ports", Nevf());
            for (const auto& key : inputData.keys()) {
                Nevf portData = inputData.getC<Nevf>(key, Nevf());
                auto id = portData.getC<unsigned int>("id", 0); 
                auto type = portData.getC<std::string>("type", ""); 
                createInputPort(key, id, type); 
            }
        }

        if (data_.contains("output_ports")) {
            Nevf outputData = data_.getC<Nevf>("output_ports", Nevf());
            for (const auto& key : outputData.keys()) {
                Nevf portData = outputData.getC<Nevf>(key, Nevf());
                auto id = portData.getC<unsigned int>("id", 0); 
                auto type = portData.getC<std::string>("type", ""); 
                createOutputPort(key, id, type); 
            }
        }
    }
};

#endif // NODE_H
