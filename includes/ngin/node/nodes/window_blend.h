#ifndef WINDOW_BLEND_H
#define WINDOW_BLEND_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nWindowBlend : public Node {
public:
    nWindowBlend(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nWindowBlend created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        isTransparent_ = lex.getC<bool>("isTransparent", true);
    }
    
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nWindowBlend with ID: " + std::to_string(getId()), 1);
        
        if (isTransparent_) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }

private:
    bool isTransparent_;

};

#endif // WINDOW_BLEND_H
