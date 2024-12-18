#ifndef SHADER_DRAWER_H
#define SHADER_DRAWER_H

#include <ngin/node/node.h>
#include <ngin/data/shader_data.h>
#include <ngin/drawer.h>

class ShaderDrawer : public Node {
public:
    explicit ShaderDrawer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
    }

    void execute(std::string& pass) override {
        Node::execute(pass); // Call base class execute to retrieve input data
        
        std::shared_ptr<NodePort> inputPortRenderGui = getInputPortByType(pass);
        if (!inputPortRenderGui) {
            return;
        }
        std::shared_ptr<ShaderData> shaderData = inputPortRenderGui->getData<ShaderData>();
        
        Drawer::render(pass, *shaderData);
    }

    void setup() override {
        Node::setup();
    }

    void launch() override {
        Node::launch();
    }
};

#endif // SHADER_DRAWER_H
