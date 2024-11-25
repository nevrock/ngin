// node_graph.h
#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>

#include <ngin/node/node.h>
#include <ngin/node/types/pass.h>
#include <ngin/node/node_port.h>
#include <ngin/node/node_connection.h>
#include <ngin/node/graph_state.h>

#include <ngin/resources.h>
#include <ngin/collections/nevf.h>

class NodeGraph {
public:
    ~NodeGraph() = default;

    explicit NodeGraph(const std::string& filepath) : data_(Resources::loadNevf(filepath)), parent_(nullptr) {
    }
    
    void build() {
        buildNodes();
        buildConnections();
        
        log();

        buildGraphStates();
    }

    // Node management
    std::shared_ptr<Node> createNode(const std::string& name, Nevf& data) {

        std::string type = data.getC<std::string>("type", "");

        if (type == "") {
            std::cerr << "no node type found." << std::endl;
            return nullptr;
        }

        auto it = getNodeFactories().find(type);
        if (it != getNodeFactories().end()) {
            std::shared_ptr<Node> node = it->second(name, data);
            node->setup();
            addNode(node);
            return node;
        } else {
            std::cerr << "unknown node type: " << type << std::endl;
            return nullptr;
        }

        return nullptr;
    }
    void addNode(std::shared_ptr<Node> node) {
        nodes_[node->getName()] = node;
    }
    void removeNode(const std::string& name) {
        auto it = nodes_.find(name);
        if (it != nodes_.end()) {
            auto node = it->second;
            nodes_.erase(it);
        }
    }
    std::shared_ptr<Node> getNode(const std::string& name) const {
        auto it = nodes_.find(name);
        return (it != nodes_.end()) ? it->second : nullptr;
    }
    std::vector<std::shared_ptr<Node>> getAllNodes() const {
        std::vector<std::shared_ptr<Node>> allNodes;
        for (const auto& [name, node] : nodes_) {
            allNodes.push_back(node);
        }
        return allNodes;
    }
    template<typename T>
    std::shared_ptr<T> getNodeByType() const {
        for (const auto& [name, node] : nodes_) {
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(node);
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }
    template<typename T>
    std::vector<std::shared_ptr<T>> getNodesByType() const {
        std::vector<std::shared_ptr<T>> matchingComponents;
        for (const auto& [name, node] : nodes_) {
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(node);
            if (casted) {
                matchingComponents.push_back(casted);
            }
        }
        return matchingComponents;
    }
    void setParent(std::shared_ptr<Node> parentIn) {
        parent_ = parentIn;
    }

    std::vector<std::shared_ptr<NodePort>> getInputPortsByName(const std::string& name) const {
        std::vector<std::shared_ptr<NodePort>> ports;
        for (const auto& [nodeName, node] : nodes_) {
            for (const auto& port : node->getInputPorts()) {
                if (port->getName() == name) {
                    ports.push_back(port);
                }
            }
        }
        return ports;
    }

    std::vector<std::shared_ptr<NodePort>> getOutputPortsByName(const std::string& name) const {
        std::vector<std::shared_ptr<NodePort>> ports;
        for (const auto& [nodeName, node] : nodes_) {
            for (const auto& port : node->getOutputPorts()) {
                if (port->getName() == name) {
                    ports.push_back(port);
                }
            }
        }
        return ports;
    }

    // Factories
    using NodeFactory = std::function<std::shared_ptr<Node>(const std::string& name, Nevf& dictionary)>;

    static std::unordered_map<std::string, NodeFactory>& getNodeFactories() {
        static std::unordered_map<std::string, NodeGraph::NodeFactory> nodeFactories;
        return nodeFactories;
    }
    static void registerNodeType(const std::string& type, NodeFactory factory) {
        getNodeFactories()[type] = factory;
    }

    void log() const {
        std::cout << "logging node graph" << std::endl;
        for (const auto& [name, node] : nodes_) {
            node->log(); // Invoke log() on each node
        }
    }


private:
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes_;
    std::unordered_map<std::string, std::shared_ptr<NodeConnection>> connections_;  // New map for storing connections
    Nevf data_; 
    std::unordered_map<std::string, GraphState> graphStates_; // Map to store graph states

    std::shared_ptr<Node> parent_;

    void buildNodes() {
        Nevf* nodes = data_.get<Nevf>("nodes", nullptr);
        if (nodes != nullptr) {
             for (const auto& key : nodes->keys()) {
                std::cout << "scene graph building object: " << key << std::endl;
                createNode(key, *nodes->get<Nevf>(key, nullptr));
            }
        }
    }

    void parseConnectionString(const std::string& input, std::string& nodeIn, std::string& portIn, std::string& nodeOut, std::string& portOut) {
        // Find the positions of the separators
        size_t firstDash = input.find('-');
        size_t tripleDash = input.find("---", firstDash + 1);
        size_t lastDash = input.find('-', tripleDash + 3);

        if (firstDash == std::string::npos || tripleDash == std::string::npos || lastDash == std::string::npos) {
            throw std::invalid_argument("invalid input format");
        }

        // Extract the parts based on positions
        nodeIn = input.substr(0, firstDash);                              // From start to first dash
        portIn = input.substr(firstDash + 1, tripleDash - firstDash - 1); // Between first dash and triple dash
        portOut = input.substr(tripleDash + 3, lastDash - tripleDash - 3); // Between triple dash and last dash
        nodeOut = input.substr(lastDash + 1);                             // After last dash
    }
    void buildParentConnection(const std::string& key, std::string& type, std::string& nodeIn, int portIn, std::string& nodeOut, int portOut) {
        if (!parent_) return;

        if (nodeIn == "parent") {
            // need to build connection between input port of parent node and
            auto outputNode = getNode(nodeOut);
            if (!outputNode) return;
            auto inputPort = parent_->getInputPortById(portIn);
            auto outputPort = outputNode->createInputPort(type, portOut, type);
            if (inputPort && outputPort) {
                // Avoid creating a connection if it already exists
                auto existingConnection = connections_.find(key);
                if (existingConnection == connections_.end()) {
                    // Create and store the connection (using a shared_ptr for memory management)
                    std::shared_ptr<NodeConnection> connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort);
                    
                    connections_[key] = connection; // Store the connection using the key
                } else {
                    std::cerr << "warning: Connection already exists: " << key << std::endl;
                }
            } else {
                std::cerr << "error: one of the ports was not found for the connection: " << key << std::endl;
            }
        } else if (nodeOut == "parent") {
            auto inputNode = getNode(nodeIn);
            if (!inputNode) return;
            auto inputPort = inputNode->createOutputPort(type, portIn, type);
            auto outputPort = parent_->createOutputPort(type, portOut, type);
            if (inputPort && outputPort) {
                // Avoid creating a connection if it already exists
                auto existingConnection = connections_.find(key);
                if (existingConnection == connections_.end()) {
                    // Create and store the connection (using a shared_ptr for memory management)
                    std::shared_ptr<NodeConnection> connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort);
                    
                    connections_[key] = connection; // Store the connection using the key
                } else {
                    std::cerr << "warning: Connection already exists: " << key << std::endl;
                }
            } else {
                std::cerr << "error: one of the ports was not found for the connection: " << key << std::endl;
            }
        }
    }
    void buildConnections() {
        Nevf* connections = data_.get<Nevf>("connections", nullptr);
        if (connections != nullptr) {
            for (const auto& key : connections->keys()) {
                std::cout << "scene graph building connection: " << key << std::endl;
                Nevf* data = connections->get<Nevf>(key, nullptr);
                std::string type = data->getC<std::string>("type", "");
                std::string nodeIn = "";
                std::string portIn = "";
                std::string nodeOut = "";
                std::string portOut = "";
                parseConnectionString(key, nodeIn, portIn, nodeOut, portOut);

                int inputPortId = std::stoi(portIn);
                int outputPortId = std::stoi(portOut);
                
                auto inputNode = getNode(nodeIn);
                auto outputNode = getNode(nodeOut);

                if (inputNode && outputNode) {
                    auto inputPort = inputNode->createOutputPort(type, inputPortId, type);
                    auto outputPort = outputNode->createInputPort(type, outputPortId, type);

                    // If both ports are now available, create the connection
                    if (inputPort && outputPort) {
                        // Avoid creating a connection if it already exists
                        auto existingConnection = connections_.find(key);
                        if (existingConnection == connections_.end()) {
                            // Create and store the connection (using a shared_ptr for memory management)
                            std::shared_ptr<NodeConnection> connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort);
                            
                            connections_[key] = connection; // Store the connection using the key
                        } else {
                            std::cerr << "warning: Connection already exists: " << key << std::endl;
                        }
                    } else {
                        std::cerr << "error: one of the ports was not found for the connection: " << key << std::endl;
                    }
                } else {
                    std::cerr << "error: one of the nodes was not found for the connection: " << key << std::endl;
                }
            }
        }
    }
    void buildGraphStates() {
        // Get all Pass nodes
        std::vector<std::shared_ptr<Pass>> passNodes = getNodesByType<Pass>(); 
        for (const auto& passNode : passNodes) {
            if (passNode != nullptr) {
                std::cout << "found pass node: " << passNode->getName() << std::endl;

                // Create and store the graph state
                graphStates_[passNode->getName()] = GraphState(); 
                graphStates_[passNode->getName()].cook(passNode); 
            }
        }
    }

};

#endif // NODE_GRAPH_H
