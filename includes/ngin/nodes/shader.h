#ifndef SUB_GRAPH_H
#define SUB_GRAPH_H

#include <ngin/data/shader_data.h>
#include <ngin/data/render_data.h>
#include <ngin/resources.h>

class Shader : public Node {
public:
    explicit Shader(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), shader_(Resources::getShaderData(dictionary.getC<std::string>("shader", ""))) {

    }

    ~Shader() override = default;

    void start(std::string& pass) override {
        Node::start(pass); // Correctly calls the base class setup(), which sets up the input ports
    
        shader_.use();

        setOutputData(pass, std::make_shared<RenderData>(pass, shader_));
    }

    void execute(std::string& pass) override {
        retrieveInputData(pass);
        update(pass);
    }
    void update(std::string& pass) override {
        Node::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        
        shader_.use();

        Log::console("shader::update " + getName());

        setOutputData(pass, std::make_shared<RenderData>(pass, shader_));
    }

    void setup() override {
        Node::setup();
    }

private:

    ShaderData& shader_;

};

#endif 