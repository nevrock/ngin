#include <ngin/node/node_graph.h>  

#include <ngin/node/nodes/window.h>
#include <ngin/node/nodes/buffer.h>
#include <ngin/node/nodes/shader.h>
#include <ngin/node/nodes/game.h>
#include <ngin/node/nodes/drawer.h>

bool window_registered = []() {
    NodeGraph::registerNode("nWindow", [](unsigned int id, const Lex& lex) {
        return std::make_unique<nWindow>(id, lex);
    });
    return true;
}();

bool buffer_registered = []() {
    NodeGraph::registerNode("nBuffer", [](unsigned int id, const Lex& lex) {
        return std::make_unique<nBuffer>(id, lex);
    });
    return true;
}();

bool shader_registered = []() {
    NodeGraph::registerNode("nShader", [](unsigned int id, const Lex& lex) {
        return std::make_unique<nShader>(id, lex);
    });
    return true;
}();

bool game_registered = []() {
    NodeGraph::registerNode("nGame", [](unsigned int id, const Lex& lex) {
        return std::make_unique<nGame>(id, lex);
    });
    return true;
}();

bool drawer_registered = []() {
    NodeGraph::registerNode("nDrawer", [](unsigned int id, const Lex& lex) {
        return std::make_unique<nDrawer>(id, lex);
    });
    return true;
}();