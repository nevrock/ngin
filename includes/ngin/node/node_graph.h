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
#include <ngin/nodes/pass.h>
#include <ngin/node/node_port.h>
#include <ngin/node/node_connection.h>
#include <ngin/node/graph_state.h>
#include <ngin/log.h>

#include <ngin/resources.h>
#include <ngin/collections/nevf.h>

class NodeGraph {
public:
    ~NodeGraph() = default;

    explicit NodeGraph(const std::string& filepath) : data_(Resources::loadNevf(filepath)), parent_(nullptr) {
    }
    
    void build() {
        Log::console("");
        Log::console("~~~NODE~GRAPH~BUILD~~~");

        //std::cout << "building node graph!" << std::endl;

        //data_.print();

        //std::cout << "-------------------" << std::endl;

        buildNodes();
        buildConnections();
        launchNodes();
        
        //log();

        buildGraphStates();
        launchGraphStates();
    }
    void execute() {
        //std::cout << "executing all graph states..." << std::endl;

        // Create a vector of pairs to sort by graphState.id
        std::vector<std::pair<std::string, GraphState>> sortedGraphStates(graphStates_.begin(), graphStates_.end());

        // Sort based on graphState.id
        std::sort(sortedGraphStates.begin(), sortedGraphStates.end(),
            [](auto& a, const auto& b) {
                return a.second.id < b.second.id;
            });

        // Execute graph states in sorted order
        for (auto& [passType, graphState] : sortedGraphStates) {
            //std::cout << "executing graph state for pass type: " << passType << ", id: " << graphState.id << std::endl;
            graphState.execute();
        }

        //std::cout << "executing complete." << std::endl;
    }

    void clean() {
        std::vector<std::pair<std::string, GraphState>> sortedGraphStates(graphStates_.begin(), graphStates_.end());

        // Sort based on graphState.id
        std::sort(sortedGraphStates.begin(), sortedGraphStates.end(),
            [](auto& a, const auto& b) {
                return a.second.id < b.second.id;
            });

        // Execute graph states in sorted order
        for (auto& [passType, graphState] : sortedGraphStates) {
            //std::cout << "executing graph state for pass type: " << passType << ", id: " << graphState.id << std::endl;
            graphState.clean();
        }
    }

    void executePass(const std::string& passType) {
        auto it = graphStates_.find(passType);
        if (it != graphStates_.end()) {
            // Retrieve the existing GraphState
            GraphState& graphState = it->second;
            graphState.execute();
        } else {
            //std::cerr << "error: graph state for pass type \"" << passType << "\" not found." << std::endl;
        }
    }

    void cleanPass(const std::string& passType) {
        auto it = graphStates_.find(passType);
        if (it != graphStates_.end()) {
            // Retrieve the existing GraphState
            GraphState& graphState = it->second;
            graphState.clean();
        } else {
            //std::cerr << "error: graph state for pass type \"" << passType << "\" not found." << std::endl;
        }
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
    std::shared_ptr<Pass> getPassNodeByType(const std::string& passType) const {
        // Retrieve all nodes of type Pass
        auto passNodes = getNodesByType<Pass>();

        // Search for the node with the matching passType
        for (const auto& passNode : passNodes) {
            if (passNode->getPass() == passType) {
                return passNode;
            }
        }
        return nullptr; // Return nullptr if no matching node is found
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
        Log::console("logging node graph!");
        for (const auto& [name, node] : nodes_) {
            node->log(); // Invoke log() on each node
        }
    }

    void buildConnection(std::string key, Nevf& data) {
        //std::cout << "node graph building connection: " << key << std::endl;
        std::string type = data.getC<std::string>("type", "");
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
                    auto connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort); 
                    refreshGraphState(type);

                    inputPort->connect(connection);
                    outputPort->connect(connection);
                    //std::shared_ptr<NodeConnection> connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort);
                    
                    connections_[key] = connection; // Store the connection using the key
                } else {
                    std::cerr << "warning: connection already exists: " << key << std::endl;
                }
            } else {
                std::cerr << "error: one of the ports was not found for the connection: " << key << std::endl;
            }
        } else {
            std::cerr << "error: one of the nodes was not found for the connection: " << key << std::endl;
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
                //std::cout << "node graph building object: " << key << std::endl;
                createNode(key, *nodes->get<Nevf>(key, nullptr));
            }
        }
    }
    void launchNodes() {
        for (const auto& [name, node] : nodes_) {
            node->launch();
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
    void buildConnections() {
        Nevf* connections = data_.get<Nevf>("connections", nullptr);
        if (connections != nullptr) {
            for (const auto& key : connections->keys()) {
                //std::cout << "node graph building connection: " << key << std::endl;
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
                            auto connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort); 

                            inputPort->connect(connection);
                            outputPort->connect(connection);
                            //std::shared_ptr<NodeConnection> connection = std::make_shared<NodeConnection>(key, type, inputPort, outputPort);
                            
                            connections_[key] = connection; // Store the connection using the key
                        } else {
                            std::cerr << "warning: connection already exists: " << key << std::endl;
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
                //std::cout << "found pass node: " << passNode->getName() << std::endl;

                // Create and store the graph state
                graphStates_[passNode->getPass()] = GraphState(passNode->getPass()); 
                graphStates_[passNode->getPass()].cook(passNode); 
            }
        }
    }
    void launchGraphStates() {
        for (auto& [passType, graphState] : graphStates_) {
            graphState.launch();
        }
    }
    void refreshGraphState(const std::string& passType) {
        // Check if a graph state exists for the given pass type
        auto it = graphStates_.find(passType);
        if (it != graphStates_.end()) {
            // Retrieve the existing GraphState
            GraphState& graphState = it->second;

            // Find the pass node corresponding to the pass type
            auto passNode = getPassNodeByType(passType);
            if (passNode) {
                //std::cout << "refreshing graph state for pass type: " << passType << std::endl;

                // Re-cook the graph state to update its structure
                graphState.cook(passNode);
            } else {
                std::cerr << "error: pass node for pass type \"" << passType << "\" not found." << std::endl;
            }
        } else {
            std::cerr << "error: graph state for pass type \"" << passType << "\" not found." << std::endl;
        }
    }

};

#endif // NODE_GRAPH_H
