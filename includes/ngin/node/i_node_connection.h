#ifndef I_NODE_CONNECTION_H
#define I_NODE_CONNECTION_H

#include <memory> // Include required headers

class NodePort; // Forward declaration of NodePort

class INodeConnection {
public:
    virtual ~INodeConnection() = default;

    virtual void transferData() = 0;

    virtual std::string& getType() = 0;

    virtual std::weak_ptr<NodePort> getInputPort() const = 0;
    virtual std::weak_ptr<NodePort> getOutputPort() const = 0;
    virtual void log() const = 0;
};

#endif // I_NODE_CONNECTION_H
