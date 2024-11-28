#ifndef I_NODE_H
#define I_NODE_H

class INode {
public:
    virtual ~INode() = default;

    // Define interface methods for Node
    virtual std::string getName() const = 0;

    virtual unsigned int getDepth() const = 0;
    virtual void setDepth(unsigned int depth) = 0;
    
    virtual std::vector<std::shared_ptr<INode>> getParentNodes(std::string type) = 0;

    virtual void execute(std::string& pass) = 0;
    virtual void setup() = 0;
    virtual void launch() = 0;
};

#endif // I_NODE_H