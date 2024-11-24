#ifndef NODE_PORT_H
#define NODE_PORT_H

#include <string>
#include <memory> // For std::shared_ptr and std::weak_ptr

class NodePort {
public:
  enum class Type {
    Input,
    Output
  };

  NodePort(const std::string& name, Type type) 
    : id_(++nextId_), name_(name), type_(type) {}
  virtual ~NodePort() = default;

  std::string getName() const { return name_; }
  int getId() const { return id_; }
  Type getType() const { return type_; }

  // Connect this port to another
  void connect(const std::shared_ptr<NodePort>& otherPort) {
    connectedPort_ = otherPort;
  }

  // Disconnect this port
  void disconnect() {
    connectedPort_.reset();
  }

  // Get the connected port (if any)
  std::shared_ptr<NodePort> getConnectedPort() const {
    return connectedPort_.lock();
  }

  // Check if the port is connected
  bool isConnected() const {
    return !connectedPort_.expired();
  }

private:
  static int nextId_; // Static variable to generate unique IDs
  int id_;
  std::string name_;
  Type type_;
  std::weak_ptr<NodePort> connectedPort_; // Use weak_ptr to avoid cycles
};

#endif // NODE_PORT_H
