#ifndef NODE_PORT_H
#define NODE_PORT_H

#include <string>
#include <memory> // For std::shared_ptr and std::weak_ptr

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
    if (linkedPort_) {
      linkedPort_->setData(data);
    }
  }
  std::shared_ptr<Nevf> getData() { return data_; }
  void clearData() { 
    data_ = nullptr;
    if (linkedPort_) {
      linkedPort_->clearData();
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
  void setLinkedPort(NodePort* port) { linkedPort_ = port; }
  void clearLinkedPort() { linkedPort_ = nullptr; }

private:

  unsigned int id_;
  std::string name_;
  std::string type_;

  std::weak_ptr<INodeConnection> connection_; 
  std::weak_ptr<INode> node_; 

  std::shared_ptr<Nevf> data_; 
  NodePort* linkedPort_ = nullptr; 

};

#endif // NODE_PORT_H