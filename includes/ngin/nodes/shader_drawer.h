#ifndef SHADER_DRAWER_H
#define SHADER_DRAWER_H

#include <ngin/nodes/node.h>
#include <ngin/data/shader_data.h>
#include <ngin/drawer.h>

class ShaderDrawer : public Node {
public:
    explicit ShaderDrawer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
    }

    void execute(std::string& pass) override {
        Node::execute(pass); // Call base class execute to retrieve input data

        // Retrieve ShaderData from input port
        auto shaderPort = getInputPortByType(pass);
        if (shaderPort) {
            auto shaderData = shaderPort->getData<ShaderData>();
            if (shaderData) {
                shaderData->use(); // Use the shader
                Drawer::render(name_, *shaderData); // Call Drawer::render with the shader
            }
        }
    }

    void setup() override {
        Node::setup();
    }

    void launch() override {
        Node::launch();
    }
};

#endif // SHADER_DRAWER_H
