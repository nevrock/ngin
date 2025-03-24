#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <ngin/node/node.h>
#include <ngin/node/node_port.h>
#include <ngin/node/node_connection.h>
#include <ngin/lex.h>
#include <ngin/resources.h>
#include <ngin/log.h> // Include Log header
#include <functional>
#include <algorithm> // For std::sort


class GraphState {
public:
    struct NodeLayer {
        unsigned int nodeId;
        int layer;

        NodeLayer(unsigned int id, int l) : nodeId(id), layer(l) {}
    };

    GraphState(unsigned int layer) : layer_(layer) {}

    void process(Node* outputNode,
                 const std::function<std::vector<NodeConnection>(unsigned int)>& getInputConnections,
                 const std::function<Node*(unsigned int)>& getNodeById) {
        if (!outputNode) {
            Log::console("Invalid output node provided for processing.", 1);
            return;
        }

        Log::console("Processing output node with ID: " + std::to_string(outputNode->getId()), 1);

        // Start with the output node at layer 0
        addOrUpdateNodeLayer(outputNode->getId(), 0);

        // Process connections iteratively
        std::vector<std::pair<Node*, int>> stack = {{outputNode, 0}};
        while (!stack.empty()) {
            auto [currentNode, currentLayer] = stack.back();
            stack.pop_back();

            auto connections = getInputConnections(currentNode->getId());
            for (const auto& connection : connections) {
                Node* inputNode = getNodeById(connection.getOutputNodeId());
                if (inputNode) {
                    int nextLayer = currentLayer + 1;
                    if (addOrUpdateNodeLayer(inputNode->getId(), nextLayer)) {
                        stack.emplace_back(inputNode, nextLayer);
                    }
                }
            }
        }

        std::sort(nodeLayers_.begin(), nodeLayers_.end(), [](const NodeLayer& a, const NodeLayer& b) {
            return a.layer > b.layer;
        });
    }

    void setup(const std::function<Node*(unsigned int)>& getNodeById,
               const std::function<std::vector<NodeConnection>(unsigned int)>& getOutputConnections,
               const std::function<void(const NodeConnection&)>& transferConnection) const {
        for (const auto& nodeLayer : nodeLayers_) {
            Node* node = getNodeById(nodeLayer.nodeId);
            if (node) {
                node->setup();
            }
            auto outputConnections = getOutputConnections(nodeLayer.nodeId);
            for (const auto& connection : outputConnections) {
                transferConnection(connection);
            }
        }
    }

    void execute(const std::function<Node*(unsigned int)>& getNodeById,
                 const std::function<std::vector<NodeConnection>(unsigned int)>& getOutputConnections,
                 const std::function<void(const NodeConnection&)>& transferConnection) const {
        for (const auto& nodeLayer : nodeLayers_) {
            Node* node = getNodeById(nodeLayer.nodeId);
            if (node) {
                node->execute();
            }
            auto outputConnections = getOutputConnections(nodeLayer.nodeId);
            for (const auto& connection : outputConnections) {
                transferConnection(connection);
            }
        }
    }

    unsigned int getLayer() const {
        return layer_;
    }

private:
    std::vector<NodeLayer> nodeLayers_;
    unsigned int layer_;

    bool addOrUpdateNodeLayer(unsigned int nodeId, int layer) {
        for (auto& nodeLayer : nodeLayers_) {
            if (nodeLayer.nodeId == nodeId) {
                if (layer > nodeLayer.layer) {
                    nodeLayer.layer = layer;
                }
                return false; // Node already exists, no need to add to stack
            }
        }
        nodeLayers_.emplace_back(nodeId, layer);
        return true; // New node added
    }
};

class NodeGraph {
public:

    NodeGraph() = default;
    ~NodeGraph() = default;

    NodeGraph(const std::string& name) {
        Log::console("Loading NodeGraph from lexicon: " + name, 1);
        Lex lex = Resources::loadLexicon(name, "node/");
        loadFromLex(lex);
        Log::console("Finished loading NodeGraph from lexicon: " + name, 1);
    }

    void setup() {
        //std::cout << "Setting up NodeGraph" << std::endl;
        //for (const auto& node : nodes_) {
        //    node->setup();
        //}
        //transfer();

        for (const auto& graphState : graphStates_) {
            Log::console("Setting up graph state: " + std::to_string(graphState.getLayer()), 1);

            graphState.setup([this](unsigned int nodeId) { return getNodeById(nodeId); },
                               [this](unsigned int nodeId) { return getOutputConnections(nodeId); },
                               [this](const NodeConnection& connection) { transferConnection(connection); });
        }
    }
    void execute() {
        //std::cout << "Executing NodeGraph" << std::endl;
        //for (const auto& node : nodes_) {
        //    node->execute();
        //}
        //transfer();
        
        for (const auto& graphState : graphStates_) {
            Log::console("Executing graph state: " + std::to_string(graphState.getLayer()), 1);

            graphState.execute([this](unsigned int nodeId) { return getNodeById(nodeId); },
                               [this](unsigned int nodeId) { return getOutputConnections(nodeId); },
                               [this](const NodeConnection& connection) { transferConnection(connection); });
        }
    }
    void transfer() {
        for (const auto& connection : connections_) {
            transferConnection(connection);
        }
    }

    void process() {
        graphStates_.clear();
        std::sort(outputNodes_.begin(), outputNodes_.end(), [](Node* a, Node* b) {
            return a->getId() < b->getId();
        });
        for (Node* outputNode : outputNodes_) {
            GraphState state(outputNode->getId());
            state.process(outputNode,
                          [this](unsigned int nodeId) { return getInputConnections(nodeId); },
                          [this](unsigned int nodeId) { return getNodeById(nodeId); });
            graphStates_.push_back(state);
        }
    }

    void deleteNode(Node* node) {
        nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(),
            [node](const std::unique_ptr<Node>& n) { return n.get() == node; }), nodes_.end());
    }

    void deleteNode(unsigned int nodeId) {
        nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(),
            [nodeId](const std::unique_ptr<Node>& n) { return n->getId() == nodeId; }), nodes_.end());
    }

    void connect(Node* outputNode, unsigned int outputPortId, Node* inputNode, unsigned int inputPortId, std::string& type) {
        Log::console("Connecting nodes: OutputNode ID " + std::to_string(outputNode->getId()) +
                     ", OutputPort ID " + std::to_string(outputPortId) +
                     " -> InputNode ID " + std::to_string(inputNode->getId()) +
                     ", InputPort ID " + std::to_string(inputPortId), 1);
        outputNode->checkOutputPort(outputPortId, type);
        inputNode->checkInputPort(inputPortId, type);
        connections_.emplace_back(outputNode->getId(), outputPortId, inputNode->getId(), inputPortId);
    }

    const std::vector<NodeConnection>& getConnections() const {
        return connections_;
    }

    void loadFromLex(const Lex& lex) {
        const auto* nodesLex = lex.get<Lex>("nodes");
        if (nodesLex) {
            for (const auto& key : nodesLex->keys()) {
                const auto* nodeLex = nodesLex->get<Lex>(key);
                if (nodeLex) {
                    std::string type = nodeLex->getC<std::string>("type", "");
                    int id = nodeLex->getC<int>("id", -1);
                    auto node = createNode(type, id, *nodeLex); // Dereference nodeLex here
                    // Load node-specific data from nodeLex if needed
                }
            }
        }

        // Sort nodes based on their IDs
        std::sort(nodes_.begin(), nodes_.end(), [](const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) {
            return a->getId() < b->getId();
        });

        outputNodes_.clear(); // Clear output nodes before processing connections

        const auto* connectionsLex = lex.get<Lex>("connections");
        if (connectionsLex) {
            for (const auto& key : connectionsLex->keys()) {
                Log::console("Processing connection key: " + key, 1);

                const auto* connectionLex = connectionsLex->get<Lex>(key);
                if (connectionLex) {
                    int outputNodeId = connectionLex->getC<int>("outputNodeId", 0);
                    int outputPortId = connectionLex->getC<int>("outputPortId", 0);
                    int inputNodeId = connectionLex->getC<int>("inputNodeId", 0);
                    int inputPortId = connectionLex->getC<int>("inputPortId", 0);
                    std::string type = connectionLex->getC<std::string>("type", "");

                    if (inputNodeId == -1) {
                        outputNodes_.push_back(getNodeById(outputNodeId));
                        Log::console("Output node found: " + std::to_string(outputNodeId), 1);

                        continue;
                    }

                    Node* outputNode = getNodeById(outputNodeId);
                    Node* inputNode = getNodeById(inputNodeId);

                    if (outputNode && inputNode) {
                        connect(outputNode, outputPortId, inputNode, inputPortId, type);
                    } else {
                        Log::console("Failed to connect nodes: " + std::to_string(outputNodeId) + " -> " + std::to_string(inputNodeId), 1);
                    }
                } else {
                    Log::console("Failed to load connection lexicon: " + key, 1);
                }
            }
        }
    }

    void transferConnection(const NodeConnection& connection) {
        Node* outputNode = getNodeById(connection.getOutputNodeId());
        Node* inputNode = getNodeById(connection.getInputNodeId());
        if (outputNode && inputNode) {
            NodePort* outputPort = outputNode->getOutputPortById(connection.getOutputPortId());
            NodePort* inputPort = inputNode->getInputPortById(connection.getInputPortId());
            if (outputPort && inputPort) {
                inputPort->transferDataFrom(outputPort);
            } else {
                Log::console("Failed to find ports for connection: " + std::to_string(connection.getOutputNodeId()) +
                             " -> " + std::to_string(connection.getInputNodeId()), 1);
            }
        } else {
            Log::console("Failed to transfer data from node: " + std::to_string(connection.getOutputNodeId()) +
                         " to node: " + std::to_string(connection.getInputNodeId()), 1);
        }
    }

    using NodeFactory = std::function<std::unique_ptr<Node>(unsigned int id, const Lex& lex)>;
    static std::unordered_map<std::string, NodeFactory>& getNodeFactories() {
        static std::unordered_map<std::string, NodeFactory> nodeFactories;
        return nodeFactories;
    }

    static void registerNode(const std::string& type, NodeFactory factory) {
        getNodeFactories()[type] = factory;
    }

    // Consolidated node creation with logging
    Node* createNode(const std::string& type, unsigned int id, const Lex& lex) {
        Log::console("Creating node of type: " + type + " with ID: " + std::to_string(id), 1);
        auto it = getNodeFactories().find(type);
        if (it != getNodeFactories().end()) {
            auto node = it->second(id, lex);
            nodes_.emplace_back(std::move(node));
            Log::console("Successfully created node of type: " + type + " with ID: " + std::to_string(id), 1);
            return nodes_.back().get();
        } else {
            Log::console("Unknown node type: " + type, 1);
            return nullptr;
        }
    }
    const std::vector<Node*>& getOutputNodes() const {
        return outputNodes_;
    }

    std::vector<NodeConnection> getConnectionsByNodeId(unsigned int nodeId) const {
        std::vector<NodeConnection> result;
        for (const auto& connection : connections_) {
            if (connection.getOutputNodeId() == nodeId || connection.getInputNodeId() == nodeId) {
                result.push_back(connection);
            }
        }
        return result;
    }

    std::vector<NodeConnection> getOutputConnections(unsigned int nodeId) const {
        std::vector<NodeConnection> result;
        for (const auto& connection : connections_) {
            if (connection.getOutputNodeId() == nodeId) {
                result.push_back(connection);
            }
        }
        return result;
    }

    std::vector<NodeConnection> getInputConnections(unsigned int nodeId) const {
        std::vector<NodeConnection> result;
        for (const auto& connection : connections_) {
            if (connection.getInputNodeId() == nodeId) {
                result.push_back(connection);
            }
        }
        return result;
    }

private:
    std::vector<std::unique_ptr<Node>> nodes_;
    std::vector<NodeConnection> connections_;
    std::vector<Node*> outputNodes_; // List of output nodes
    std::vector<GraphState> graphStates_;

    Node* getNodeById(unsigned int id) {
        for (const auto& node : nodes_) {
            if (node->getId() == id) {
                return node.get();
            }
        }
        return nullptr;
    }
};

#endif // NODE_GRAPH_H
