#ifndef DRAWER_NODE_H
#define DRAWER_NODE_H

#include <ngin/node/node.h>
#include <ngin/drawer.h>
#include <ngin/log.h>

#include <iostream>

class nDrawer : public Node {
public:
    nDrawer(unsigned int id, const Lex& lex) : passName_(lex.getC<std::string>("pass", "")) {
        setId(id);
        loadFromLex(lex);
    }
    void setup() override {
    }
    void execute() override {
        Log::console("Executing nDrawer with ID: " + std::to_string(getId()), 1);

        // Add WindowNode-specific execution logic here
        Drawer::prep(passName_);
        Drawer::draw(passName_);
    }
private:
    std::string passName_;
};

#endif // BUFFER_NODE_H
