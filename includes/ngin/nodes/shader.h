#ifndef SUB_GRAPH_H
#define SUB_GRAPH_H

#include <ngin/data/shader_data.h>
#include <ngin/resources.h>

class Shader : public Node {
public:
    explicit Shader(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary)
    {
        shader_ = Resources::loadShaderData(dictionary.getC<std::string>("shader", ""));
    }

    ~Shader() override = default;

    void execute(std::string& pass) override {
        Node::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        
        // TODO: Communicate pointer to output port
        shader_->use();

        for (const auto& port : outputPorts_) {
            if (port->getName().find("render") != std::string::npos) {
                port->setData<ShaderData>(shader_);
                //std::cout << "shader execute and set data: " << port->getName() << std::endl;
            }
        }
    }

    void setup() override {
        Node::setup();
    }

    std::shared_ptr<ShaderData> getData() {
        return shader_;
    }

private:

    std::shared_ptr<ShaderData> shader_;

};

#endif 