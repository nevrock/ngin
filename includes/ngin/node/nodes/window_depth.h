#ifndef WINDOW_DEPTH_H
#define WINDOW_DEPTH_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nWindowDepth : public Node {
public:
    nWindowDepth(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nWindowDepth created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        isDepth_ = lex.getC<bool>("isDepth", true);
    }
    
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nWindowDepth with ID: " + std::to_string(getId()), 1);
        
        if (isDepth_) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
    }

private:
    bool isDepth_;

};

#endif // WINDOW_DEPTH_H
