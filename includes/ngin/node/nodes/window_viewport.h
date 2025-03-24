#ifndef WINDOW_VIEWPORT_H
#define WINDOW_VIEWPORT_H

#include <iostream>
#include <glad/glad.h>

#include <ngin/node/node.h>
#include <ngin/log.h>


class nWindowViewport : public Node {
public:
    nWindowViewport(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
        Log::console("nWindowViewport created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        width_ = lex.getC<int>("width", 800);
        height_ = lex.getC<int>("height", 600);
    }
    
    void setup() override {
        
    }
    void execute() override {
        Log::console("Executing nWindowViewport with ID: " + std::to_string(getId()), 1);
        
        glViewport(0, 0, width_, height_);
    }

private:
    int width_, height_;

};

#endif // WINDOW_VIEWPORT_H
