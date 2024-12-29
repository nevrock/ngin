#ifndef GL_BSDF_H
#define GL_BSDF_H

#include <GLFW/glfw3.h>
#include <ngin/node/node.h>

class GlBsdf : public Node {
public:
    GlBsdf(const std::string& name, Nevf& data) : Node(name, data) {
    }

    // Override the execute method
    void update(std::string& pass) override {
        Node::update(pass); // retrieve inputs

        int screenWidth = Game::env<int>("screen.width");
        int screenHeight = Game::env<int>("screen.height");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void setup() override {
        Node::setup();
    }


private:
};

#endif // GL_SHADOWS_POINT_H