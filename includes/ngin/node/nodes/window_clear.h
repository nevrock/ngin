#ifndef WINDOW_CLEAR_H
#define WINDOW_CLEAR_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nWindowClear : public Node {
public:
    nWindowClear(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nWindowClear created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        isClearColor_ = lex.getC<bool>("isClearColor", true);
    }
    
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nWindowClear with ID: " + std::to_string(getId()), 1);
        
        if (isClearColor_)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

private:
    bool isClearColor_;

};

#endif // WINDOW_CLEAR_H
