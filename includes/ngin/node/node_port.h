#ifndef NODE_PORT_H
#define NODE_PORT_H

#include <string>
#include <memory> // For std::shared_ptr and std::weak_ptr
#include <vector> // For std::vector
#include <algorithm> // For std::remove_if

#include <ngin/collections/nevf.h>
#include <ngin/node/i_node_connection.h>
#include <ngin/node/i_node.h>

class NodePort {
public:
  NodePort(const std::string& name, unsigned int id, const std::string& type, std::weak_ptr<INode> node) 
    : id_(id), name_(name), type_(type), node_(node) {}
  virtual ~NodePort() = default;

  void setData(std::shared_ptr<Nevf> data) { 
    data_ = data; 
    for (const auto& weakPort : linkedPorts_) {
      if (auto port = weakPort.lock()) {
        port->setData(data);
      }
    }
  }

  std::shared_ptr<Nevf> getData() { return data_; }

  void clearData() { 
    data_ = nullptr;
    for (const auto& weakPort : linkedPorts_) {
      if (auto port = weakPort.lock()) {
        port->clearData();
      }
    }
  }

  std::string getName() const { return name_; }
  unsigned int getId() const { return id_; }

  std::weak_ptr<INode> getNode() {
    return node_;
  }

  void connect(std::weak_ptr<INodeConnection> connection) {
    connection_ = connection;
  }

  void disconnect() {
    connection_.reset(); // Use reset() to clear weak_ptr
  }

  std::weak_ptr<INodeConnection> getConnection() const {
    return connection_;
  }

  bool isConnected() const {
    return !connection_.expired(); // Check if the connection is still alive
  }

  // Methods for managing linked ports
  void addLinkedPort(std::weak_ptr<NodePort> port) {
    if (auto strongPort = port.lock()) {
      if (std::find_if(linkedPorts_.begin(), linkedPorts_.end(), 
          [&strongPort](const std::weak_ptr<NodePort>& p) { return p.lock() == strongPort; }) == linkedPorts_.end()) {
        linkedPorts_.push_back(port);
      }
    }
  }

  void clearLinkedPorts() {
    linkedPorts_.clear();
  }

  void removeLinkedPort(std::weak_ptr<NodePort> port) {
    linkedPorts_.erase(
      std::remove_if(linkedPorts_.begin(), linkedPorts_.end(),
        [&port](const std::weak_ptr<NodePort>& p) { return p.lock() == port.lock(); }),
      linkedPorts_.end()
    );
  }

private:
  unsigned int id_;
  std::string name_;
  std::string type_;

  std::weak_ptr<INodeConnection> connection_; 
  std::weak_ptr<INode> node_; 

  std::shared_ptr<Nevf> data_; 
  std::vector<std::weak_ptr<NodePort>> linkedPorts_; // Collection of linked ports
};

#endif // NODE_PORT_H
