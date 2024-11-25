#ifndef NODE_PORT_H
#define NODE_PORT_H

#include <string>
#include <memory> // For std::shared_ptr and std::weak_ptr

#include <ngin/collections/nevf.h>

#include <ngin/node/i_node_connection.h>
#include <ngin/node/i_node.h>


class NodePort {
public:
  NodePort(const std::string& name, unsigned int id, const std::string& type, INode* node) 
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
  } // Add this line


  std::string getName() const { return name_; }
  unsigned int getId() const { return id_; }

  INode* getNode() {
    return node_;
  }


  void connect(INodeConnection* connection) {
    connection_ = connection;
  }
  void disconnect() {
    connection_ = nullptr;
  }
  INodeConnection* getConnection() const {
    return connection_;
  }
  bool isConnected() const {
    return connection_ != nullptr;
  }
  void setLinkedPort(NodePort* port) { linkedPort_ = port; }
  void clearLinkedPort() { linkedPort_ = nullptr; }

private:

  unsigned int id_;
  std::string name_;
  std::string type_;

  //bool isConnected_;

  INodeConnection* connection_; // Use weak_ptr to avoid cycles
  INode* node_;

  std::shared_ptr<Nevf> data_; // Pointer to the Nevf instance
  NodePort* linkedPort_ = nullptr; // Pointer to the linked port

};

#endif // NODE_PORT_H
