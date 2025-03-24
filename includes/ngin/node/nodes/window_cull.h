#ifndef WINDOW_CULL_H
#define WINDOW_CULL_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nWindowCull : public Node {
public:
    nWindowCull(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nWindowCull created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        isCull_ = lex.getC<bool>("isCull", true);
        isFront_ = lex.getC<bool>("isFront", true);
    }
    
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nWindowCull with ID: " + std::to_string(getId()), 1);
        
        if (isCull_) {
            glEnable(GL_CULL_FACE);
            if (isFront_) {
                glCullFace(GL_FRONT);
            } else {
                glCullFace(GL_BACK);
            }
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

private:
    bool isCull_;
    bool isFront_;

};

#endif // WINDOW_CULL_H
