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

    void update(std::string& pass) override {
        Node::update(pass); // Call base class execute to retrieve input data
        
        std::shared_ptr<NodePort> inputPortRenderGui = getInputPortByType(pass);
        if (!inputPortRenderGui) {
            Log::console("No input port found for pass: " + pass);
            return;
        }
        std::shared_ptr<RenderData> renderData = inputPortRenderGui->getData<RenderData>();
        if (renderData) {
            Drawer::render(pass, renderData->getShader());
        } else {
            Log::console("No shader data found for pass: " + pass);
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
