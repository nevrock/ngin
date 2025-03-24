#ifndef NODE_CONNECTION_H
#define NODE_CONNECTION_H

class NodeConnection {
public:
    NodeConnection(unsigned int outputNodeId, unsigned int outputPortId, unsigned int inputNodeId, unsigned int inputPortId)
        : outputNodeId_(outputNodeId), outputPortId_(outputPortId), inputNodeId_(inputNodeId), inputPortId_(inputPortId) {}

    unsigned int getOutputNodeId() const { return outputNodeId_; }
    unsigned int getOutputPortId() const { return outputPortId_; }
    unsigned int getInputNodeId() const { return inputNodeId_; }
    unsigned int getInputPortId() const { return inputPortId_; }

private:
    unsigned int outputNodeId_;
    unsigned int outputPortId_;
    unsigned int inputNodeId_;
    unsigned int inputPortId_;
};

#endif // NODE_CONNECTION_H
