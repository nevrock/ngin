#ifndef GL_FRAMEBUFFER_H
#define GL_FRAMEBUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/node/node.h>
#include <ngin/game.h>

class GlFramebuffer : public Node {
public:
    explicit GlFramebuffer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
    }

    ~GlFramebuffer() override = default;

    void update(std::string& pass) override {
        Node::update(pass);
    }

    void start(std::string& pass) override {
        Node::start(pass);

        // pbr: setup framebuffer
        // ----------------------
        glGenFramebuffers(1, &captureFBO_);
        glGenRenderbuffers(1, &captureRBO_);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO_);

        Game::envset("captureFBO", captureFBO_);
        Game::envset("captureRBO", captureRBO_);
    }

private:
    unsigned int captureFBO_, captureRBO_;
};

#endif // GL_FRAMEBUFFER_H
