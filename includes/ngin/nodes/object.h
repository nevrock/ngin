#ifndef OBJECT_H
#define OBJECT_H

#include <ngin/node/node_graph.h>

class Object : public Node {
public:
    explicit Object(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary)
    {
    }

    ~Object() override = default;

    void execute(std::string& pass) override {
        Node::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        graph_->executePass(pass);
    }

    void setup() override {
        Node::setup();

        if (data_.contains("file")) {
            graph_ = std::make_unique<NodeGraph>(data_.getC<std::string>("file", ""));
            graph_->setParent(Node::shared_from_this());
            graph_->build();
        }
    }

    void launch() override {
        preprocessGraph();
    }

protected:
    std::unique_ptr<NodeGraph> graph_;

    void preprocessGraph() {
        std::vector<std::shared_ptr<NodePort>> inputPorts = graph_->getInputPortsByName("parent");
        std::vector<std::shared_ptr<NodePort>> outputPorts = graph_->getOutputPortsByName("parent");

        //std::cout << "sub graph has INPUT port count: " << inputPorts.size() << ", sub graph has OUTPUT port count: " << outputPorts.size() << std::endl;

        // Iterate through input ports
        for (const auto& graphInputPort : inputPorts) {
            std::shared_ptr<NodePort> objectInputPort = getInputPortById(graphInputPort->getId());
            if (objectInputPort) {
                //std::cout << "adding linked port, from: " << objectInputPort->getName() << " to " << graphInputPort->getName() << std::endl;
                objectInputPort->addLinkedPort(graphInputPort); 
            } else {
                //std::cout << "adding linked port failed, with id: " << graphInputPort->getId() << std::endl;
                //std::cout << "current port has ids: " << "(length: " << inputPorts_.size() << ")" << std::endl;
                //for (const auto& port : inputPorts_) {
                //    std::cout << port->getId() << std::endl;
                //}
            }
        }

        // Iterate through output ports
        for (const auto& graphOutputPort : outputPorts) {
            std::shared_ptr<NodePort> objectOutputPort = getOutputPortById(graphOutputPort->getId());
            if (objectOutputPort) {
                graphOutputPort->addLinkedPort(objectOutputPort); 
            }
        }

        //std::cout << "--- object graph has input ports: " << inputPorts.size() 
        //          << ", " << outputPorts.size() << std::endl;
    }
};

#endif // OBJECT_GRAPH_H