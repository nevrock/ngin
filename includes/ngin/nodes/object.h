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

    void update(std::string& pass) override {
        Node::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        
        std::shared_ptr<NodePort> inputPort = getInputPortByType(pass);
        if (inputPort) {
            std::shared_ptr<IData> data = inputPort->getRawData();
            if (data) {
               // Log::console("object: " + getName() + " received data for pass: " + pass);
            } else {
                //Log::console("object: " + getName() + " did not receive data for pass: " + pass);
            }
        } else {
            //Log::console("object: " + getName() + " did not receive data for pass: " + pass);
        }
        
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

        Log::console("preprocessing graph with " + std::to_string(inputPorts.size()) + " input ports and " + std::to_string(outputPorts.size()) + " output ports");

        // Iterate through input ports
        for (const auto& graphInputPort : inputPorts) {
            std::shared_ptr<NodePort> objectInputPort = getInputPortById(graphInputPort->getId());
            if (objectInputPort) {
                objectInputPort->addLinkedPort(graphInputPort); 
            }
        }

        // Iterate through output ports
        for (const auto& graphOutputPort : outputPorts) {
            std::shared_ptr<NodePort> objectOutputPort = getOutputPortById(graphOutputPort->getId());
            if (objectOutputPort) {
                graphOutputPort->addLinkedPort(objectOutputPort); 
            }
        }
    }
};

#endif // OBJECT_GRAPH_H