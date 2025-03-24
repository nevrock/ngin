#ifndef GAME_NODE_H
#define GAME_NODE_H

#include <ngin/node/node.h>
#include <ngin/game.h>
#include <ngin/log.h>

#include <iostream>

class nGame : public Node {
public:
    nGame(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
    }
    void setup() override {
        Game::start();
    }
    void execute() override {
        Log::console("Executing nGame with ID: " + std::to_string(getId()), 1);
        // Add WindowNode-specific execution logic here
        
        // Update game logic and transformations
        Game::updateLogic();
        Game::updateTransform();
    }
private:
};

#endif // BUFFER_NODE_H
