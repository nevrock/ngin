// node_graph.h
#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include <ngin/node/node.h>
#include <ngin/node/node_port.h>

#include <ngin/resources.h>
#include <ngin/collections/nevf.h>

class NodeGraph {
public:
    ~NodeGraph() = default;

    explicit NodeGraph(const std::string& filepath) : data_(Resources::loadNevf(filepath)) {}

    // Add a new node to the graph
    std::shared_ptr<Node> createNode(const std::string& name, Nevf& data) {
        auto node = std::make_shared<Node>(name, data);
        addNode(node);
        node->setGetNodeByNameFunc([this](const std::string& name) {
            return getNode(name); 
        });
        return node;
    }
    // Add a node
    void addNode(std::shared_ptr<Node> node) {
        nodes_[node->getName()] = node;
        nodesByDepth_[node->depth].push_back(node);
    }
    // Remove a node by name
    void removeNode(const std::string& name) {
        auto it = nodes_.find(name);
        if (it != nodes_.end()) {
            auto node = it->second;
            auto& depthNodes = nodesByDepth_[node->depth];
            // Remove the node from the depth map
            depthNodes.erase(
                std::remove(depthNodes.begin(), depthNodes.end(), node),
                depthNodes.end()
            );
            if (depthNodes.empty()) {
                nodesByDepth_.erase(node->depth);
            }
            nodes_.erase(it);
        }
    }
    // Find a node by name
    std::shared_ptr<Node> getNode(const std::string& name) const {
        auto it = nodes_.find(name);
        return (it != nodes_.end()) ? it->second : nullptr;
    }
    // Get all nodes
    std::vector<std::shared_ptr<Node>> getAllNodes() const {
        std::vector<std::shared_ptr<Node>> allNodes;
        for (const auto& [name, node] : nodes_) {
            allNodes.push_back(node);
        }
        return allNodes;
    }
    // Get all nodes at a specific depth
    std::vector<std::shared_ptr<Node>> getNodesByDepth(int depth) const {
        auto it = nodesByDepth_.find(depth);
        if (it != nodesByDepth_.end()) {
            return it->second;
        }
        return {};
    }

protected:
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes_; // Map of node names to nodes
    std::unordered_map<int, std::vector<std::shared_ptr<Node>>> nodesByDepth_; // Map of depth to nodes
    Nevf data_; // Loaded data
};

#endif // NODE_GRAPH_H
