#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory> // For std::shared_ptr
#include <functional> // For std::function
#include <algorithm> // For std::find_if

#include <ngin/node/node_port.h>

class Node {
public:
  unsigned int depth;

  Node(const std::string& name, Nevf& dictionary) : name_(name), data_(dictionary) {
    
  }
  virtual ~Node() = default;

  void buildConnections() {
    // TODO: setup ports from data_ dictionary
    if (data_.contains("inputs")) {
        Nevf inputs = data_.get("inputs");
        for (auto& inputKey : inputs.keys()) {
            Nevf inputData = inputs.get(inputKey);
            // Create NodePort using inputData (type, connection, etc.)
            auto inputPort = std::make_shared<NodePort>(/* ... */); 
            addInputPort(inputPort);
        }
    }

    if (data_.contains("outputs")) {
        Nevf outputs = data_.get("outputs");
        for (auto& outputKey : outputs.keys()) {
            Nevf outputData = outputs.get(outputKey);
            // Create NodePort using outputData (type, connection, etc.)
            auto outputPort = std::make_shared<NodePort>(/* ... */); 
            addOutputPort(outputPort);
        }
    }
  }

  std::string getName() const { return name_; }

  // Add an input port to the node
  void addInputPort(const std::shared_ptr<NodePort>& port) { inputPorts_.push_back(port); }
  // Add an output port to the node
  void addOutputPort(const std::shared_ptr<NodePort>& port) { outputPorts_.push_back(port); }
  // Get input ports
  std::vector<std::shared_ptr<NodePort>>& getInputPorts() { return inputPorts_; }
  // Get output ports
  std::vector<std::shared_ptr<NodePort>>& getOutputPorts() { return outputPorts_; }

  // Get input port by ID
  std::shared_ptr<NodePort> getInputPortById(int id) {
    auto it = std::find_if(inputPorts_.begin(), inputPorts_.end(),
                            [id](const std::shared_ptr<NodePort>& port) { return port->getId() == id; });
    return (it != inputPorts_.end()) ? *it : nullptr;
  }

  // Get input port by name
  std::shared_ptr<NodePort> getInputPortByName(const std::string& name) {
    auto it = std::find_if(inputPorts_.begin(), inputPorts_.end(),
                            [&name](const std::shared_ptr<NodePort>& port) { return port->getName() == name; });
    return (it != inputPorts_.end()) ? *it : nullptr;
  }

  // Get output port by ID
  std::shared_ptr<NodePort> getOutputPortById(int id) {
    auto it = std::find_if(outputPorts_.begin(), outputPorts_.end(),
                            [id](const std::shared_ptr<NodePort>& port) { return port->getId() == id; });
    return (it != outputPorts_.end()) ? *it : nullptr;
  }

  // Get output port by name
  std::shared_ptr<NodePort> getOutputPortByName(const std::string& name) {
    auto it = std::find_if(outputPorts_.begin(), outputPorts_.end(),
                            [&name](const std::shared_ptr<NodePort>& port) { return port->getName() == name; });
    return (it != outputPorts_.end()) ? *it : nullptr;
  }

  void setGetNodeByNameFunc(std::function<std::shared_ptr<Node>(const std::string&)> func) {
    getNodeByName_ = func;
  }

  std::shared_ptr<Node> getNeighbor(const std::string& neighborName) {
      if (getNodeByName_) {
          return getNodeByName_(neighborName);
      } else {
          return nullptr; // Or throw an exception
      }
  }

  // Execute the node's logic (default implementation does nothing)
  virtual void execute() {
    // Default implementation: No operation
  }

protected:
  std::string name_;
  std::vector<std::shared_ptr<NodePort>> inputPorts_;
  std::vector<std::shared_ptr<NodePort>> outputPorts_;
  std::function<std::shared_ptr<Node>(const std::string&)> getNodeByName_;

  Nevf& data_; 
};

#endif // NODE_H