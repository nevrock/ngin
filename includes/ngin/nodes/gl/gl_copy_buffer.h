#ifndef GL_COPY_BUFFER_H
#define GL_COPY_BUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/node/node.h>
#include <ngin/game.h>

class GlCopyBuffer : public Node {
public:
    explicit GlCopyBuffer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
    }

    ~GlCopyBuffer() override = default;

    void update(std::string& pass) override {
        Node::update(pass);
        unsigned int gBuffer = Game::env<unsigned int>("g_buffer");
        int screenWidth = Game::env<int>("screen.width");
        int screenHeight = Game::env<int>("screen.height");

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void start(std::string& pass) override {
        Node::start(pass);
    }

private:
};

#endif // GL_COPY_BUFFER_H
