#ifndef OBJECT_GRAPH_H
#define OBJECT_GRAPH_H

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

        if (pass == "transform") {
            // update transform data
            std::shared_ptr<NodePort> port = getInputPortByName("transform");
            if (port->isConnected()) {

            }
        }
    }

    void setup() override {
        Node::setup();
        //std::cout << "object setup!" << std::endl;
        //std::cout << data_.getString() << std::endl;
        if (data_.contains("graph")) {
            graph_ = std::make_unique<NodeGraph>("assets/graphs/" + data_.getC<std::string>("graph", ""));
            graph_->setParent(Node::shared_from_this());
            graph_->build();
        }
    }

protected:
    std::unique_ptr<NodeGraph> graph_;

    void preprocessGraph() {
        
    }
};

#endif // OBJECT_GRAPH_H