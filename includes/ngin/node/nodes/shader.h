#ifndef SHADER_NODE_H
#define SHADER_NODE_H

#include <ngin/node/node.h>
#include <ngin/resources.h>
#include <ngin/log.h>

#include <iostream>

class nShader : public Node {
public:
    nShader(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);
        // Load additional properties here
        shaderName_ = lex.getC<std::string>("shader", "");
    }
    void setup() override {
        ShaderData& shaderSsao = Resources::getShaderData(shaderName_);

        NodePort* outputPort = getOutputPortByType("shader");

        if (outputPort) { 
            outputPort->setVar("shader", &shaderSsao);
        } 
    }
    void execute() override {
        Log::console("Executing nShader with ID: " + std::to_string(getId()), 1);
        // Add WindowNode-specific execution logic here
    }
private:
    std::string shaderName_;
};

#endif // BUFFER_NODE_H
