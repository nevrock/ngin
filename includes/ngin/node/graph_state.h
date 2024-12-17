#ifndef GRAPHSTATE_H
#define GRAPHSTATE_H

#include <unordered_map>
#include <memory> // For std::shared_ptr
#include <queue> // For std::queue

#include <ngin/node/i_node.h>

class GraphState {
public:

    GraphState(std::string type) : type_(type) {}
    GraphState() {} // Add a default constructor
    ~GraphState() {}

    unsigned int id;

    std::string getType() {
        return type_;
    }

    void cook(std::shared_ptr<INode> node) {
        root_ = node;

        id = node->getId();

        totalDepth_ = 0;
        // 1. Set all depthTemp values to -1
        resetDepth(root_); 

        // 2. Start at root, set depth to 0
        root_->setDepth(0);
        //root_->depthTemp = 0; 

        // 3. Recursively calculate depths
        calculateDepth(root_); 

        // 4. Populate nodes_ map (optional, if you still need it)
        populateNodesMap(root_); 

        printDepthLayers();
    }

    unsigned int getTotalDepth() const {
        return totalDepth_;
    }

    std::vector<std::shared_ptr<INode>> getNodesByDepth(int depth) const {
        auto it = nodes_.find(depth);
        if (it != nodes_.end()) {
            return it->second; // Return the vector of nodes at this depth
        }
        return {}; // Return an empty vector if the depth doesn't exist
    }

    void execute() {
        // Iterate from totalDepth down to 0
        for (int depth = totalDepth_; depth >= 0; --depth) {
            // Get nodes at the current depth
            std::vector<std::shared_ptr<INode>> nodesAtDepth = getNodesByDepth(depth); 

            // Execute each node at the current depth
            for (const auto& node : nodesAtDepth) {
                node->execute(type_); // Pass passType as an argument
            }
        }
    }


private:
    void resetDepth(std::shared_ptr<INode> node) {
        node->setDepth(-1);
        for (const auto& parent : node->getParentNodes(type_)) {
            resetDepth(parent);
        }
    }
    void calculateDepth(std::shared_ptr<INode> node) {
        unsigned int depthC = 0;
        for (const auto& parent : node->getParentNodes(type_)) {
            if (parent->getDepth() == -1) { // Not visited yet
                depthC = node->getDepth() + 1;
                if (depthC > totalDepth_) {
                    totalDepth_ = depthC;
                }
                parent->setDepth(depthC); 
                calculateDepth(parent); 
            } else {
                // Ensure the current depth is less deep than the already set depth
                if (node->getDepth() + 1 > parent->getDepth()) {
                    depthC = node->getDepth() + 1;
                    if (depthC > totalDepth_) {
                        totalDepth_ = depthC;
                    }
                    parent->setDepth(depthC);
                    calculateDepth(parent); // Recalculate depths for the parent
                }
            }
        }
    }
    void populateNodesMap(std::shared_ptr<INode> node) {
        auto& nodesAtDepth = nodes_[node->getDepth()];
        // Check if a node with the same name already exists at this depth
        auto it = std::find_if(nodesAtDepth.begin(), nodesAtDepth.end(),
                            [&node](const std::shared_ptr<INode>& n) {
                                return n->getName() == node->getName();
                            });
        if (it == nodesAtDepth.end()) {
            // Only add the node if no duplicate found
            nodesAtDepth.push_back(node);
        }
        // Recursively process parent nodes
        for (const auto& parent : node->getParentNodes(type_)) {
            populateNodesMap(parent);
        }
    }
    void printDepthLayers() const {
        Log::console("-------------------");
        Log::console("logging graph state!");
        for (const auto& [depth, nodes] : nodes_) {
            for (const auto& node : nodes) {
                Log::console(" - " + node->getName(), depth);
            }
        }
    }

    std::shared_ptr<INode> root_;
    std::string type_;
    std::unordered_map<int, std::vector<std::shared_ptr<INode>>> nodes_; // Changed to map of int and vector of nodes
    unsigned int totalDepth_;
};

#endif // GRAPHSTATE_H