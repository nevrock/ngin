#ifndef BUFFER_NODE_H
#define BUFFER_NODE_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nBuffer : public Node {
public:
    nBuffer(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nBuffer created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        fbo_ = lex.getC<int>("fbo", 0);
    }
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nBuffer with ID: " + std::to_string(getId()), 1);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    }

private:
    unsigned int fbo_;

};

#endif // BUFFER_NODE_H
