#ifndef TEXTURES_NODE_H
#define TEXTURES_NODE_H

#include <ngin/node/node.h>
#include <ngin/window.h>
#include <ngin/log.h>

#include <iostream>
#include <glad/glad.h>

class nTextureBind : public Node {
public:
    nTextureBind(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nTextureBind created with ID: " + std::to_string(id), 1);

        addInputPort(0, "int");
        addOutputPort(1, "int");
    }
    void setup() override {
        // two input ports

        NodePort* inputPortTextureLocation = getInputPortById(0);
        NodePort* inputPortTextureId = getInputPortById(1);

        if (inputPortTextureLocation) {
            textureLocation_ = inputPortTextureLocation->getVar<int>("value", 0);
        }
        if (inputPortTextureId) {
            textureId_ = inputPortTextureId->getVar<int>("value", 0);
        }
    }
    void execute() override {
        Log::console("Executing nTextureBind with ID: " + std::to_string(getId()), 1);

        // Add WindowNode-specific execution logic here
        glActiveTexture(GL_TEXTURE+textureLocation_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
    }

private:
    unsigned int textureId_;
    unsigned int textureLocation_;
        
};

#endif // TEXTURES_NODE_H
