#ifndef SUB_GRAPH_H
#define SUB_GRAPH_H

class SubGraph : public Node {
public:
    explicit SubGraph(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary)
    {
    }

    ~SubGraph() override = default;

    void execute(std::string& pass) override {
        Node::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    }

    void setup() override {
        Node::setup();
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> getNodesByType(bool deepSearch = false) const {
        return {}; // Return an empty vector
    }

protected:
};

#endif 