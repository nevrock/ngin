#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>
#include <ngin/node/node_port.h>
#include <ngin/lex.h> // Assuming Lex is defined here

class Node {
public:
    virtual ~Node() = default;
    virtual void setup() = 0;
    virtual void execute() = 0;
    virtual void end() = 0;

    virtual void loadFromLex(const Lex& lex) { // Marked as virtual
        loadPorts(lex.getC<Lex>("inputPorts", Lex()), lex.getC<Lex>("outputPorts", Lex()));
    }
    void loadPorts(const Lex& inputPortsLex, const Lex& outputPortsLex) {
        for (const auto& key : inputPortsLex.keys()) {
            const Lex* inputPort = inputPortsLex.get<Lex>(key);
            if (inputPort) {
                unsigned int id = inputPort->getC<unsigned int>("id", 0);
                std::string type = inputPort->getC<std::string>("type", "");
                addInputPort(id, type);
            }
        }
        for (const auto& key : outputPortsLex.keys()) {
            const Lex* outputPort = outputPortsLex.get<Lex>(key);
            if (outputPort) {
                unsigned int id = outputPort->getC<unsigned int>("id", 0);
                std::string type = outputPort->getC<std::string>("type", "");
                addOutputPort(id, type);
            }
        }
    }

    void addInputPort(unsigned int id) {
        inputPorts_.emplace_back(std::make_unique<NodePort>(id));
    }

    void addOutputPort(unsigned int id) {
        outputPorts_.emplace_back(std::make_unique<NodePort>(id));
    }

    const std::vector<std::unique_ptr<NodePort>>& getInputPorts() const {
        return inputPorts_;
    }

    const std::vector<std::unique_ptr<NodePort>>& getOutputPorts() const {
        return outputPorts_;
    }

    NodePort* getInputPortById(unsigned int id) const {
        for (const auto& port : inputPorts_) {
            if (port->getId() == id) {
                return port.get();
            }
        }
        return nullptr;
    }

    NodePort* getOutputPortById(unsigned int id) const {
        for (const auto& port : outputPorts_) {
            if (port->getId() == id) {
                return port.get();
            }
        }
        return nullptr;
    }

    void checkInputPort(unsigned int id, const std::string& type) {
        NodePort* port = getInputPortById(id);
        if (port == nullptr) {
            addInputPort(id, type);
        }
    }
    void addInputPort(unsigned int id, const std::string& type) {
        inputPorts_.emplace_back(std::make_unique<NodePort>(id, type));
    }

    void checkOutputPort(unsigned int id, const std::string& type) {
        NodePort* port = getOutputPortById(id);
        if (port == nullptr) {
            addOutputPort(id, type);
        }
    }
    void addOutputPort(unsigned int id, const std::string& type) {
        outputPorts_.emplace_back(std::make_unique<NodePort>(id, type));
    }

    NodePort* getInputPortByType(const std::string& type) const {
        for (const auto& port : inputPorts_) {
            if (port->getType() == type) {
                return port.get();
            }
        }
        return nullptr;
    }
    NodePort* getOutputPortByType(const std::string& type) const {
        for (const auto& port : outputPorts_) {
            if (port->getType() == type) {
                return port.get();
            }
        }
        return nullptr;
    }

    unsigned int getId() const { return id_; }
    void setId(unsigned int id) { id_ = id; }

private:
    unsigned int id_;
    std::vector<std::unique_ptr<NodePort>> inputPorts_;
    std::vector<std::unique_ptr<NodePort>> outputPorts_;
};

#endif // NODE_H
