#ifndef POST_PASS_H
#define POST_PASS_H

#include <ngin/render/render_pass.h>
#include <iostream>

class PostPass : public RenderPass {
public:
    PostPass(unsigned int id) : RenderPass(id) {}

    void setup() override {
        std::cout << "Setting up post pass with ID: " << getId() << std::endl;

        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        // Generate framebuffer
        glGenFramebuffers(1, &postFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, postFBO_);

        // Generate texture to render to
        glGenTextures(1, &postTexture_);
        glBindTexture(GL_TEXTURE_2D, postTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postTexture_, 0);

        // Create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &postRBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, postRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, postRBO_);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Log::console("Post-processing framebuffer not complete!", 1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void render() override {

    }
    void bind() override {
    }

private:
    unsigned int postFBO_, postRBO_, postTexture_;

};

#endif // POST_PASS_H
