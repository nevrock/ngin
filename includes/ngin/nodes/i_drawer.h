#ifndef I_DRAWER_H
#define I_DRAWER_H

#include <ngin/node/node.h>
#include <ngin/data/shader_data.h>

class IDrawer : public Node, public std::enable_shared_from_this<IDrawer> {
public:
    using Node::Node; // Inherit constructors from Node

    virtual void render(std::shared_ptr<ShaderData> shader) = 0; // Pure virtual method for rendering

    void setup() override {
        Node::setup();
    }
    void launch() override {
        Node::launch();
    }
    void execute(std::string& pass) override {
        Node::execute(pass);
    }
    void update(std::string& pass) override {
        Node::update(pass);
    }
};

#endif // I_DRAWER_H
