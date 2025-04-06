#ifndef POST_PASS_H
#define POST_PASS_H

#include <ngin/render/render_pass.h>
#include <iostream>

class AAPass : public RenderPass {
public:
    AAPass(unsigned int id) : RenderPass(id) {}

    void setup() override {
        std::cout << "Setting up post pass with ID: " << getId() << std::endl;

        screenWidth_ = Ngin::envget<int>("screen.width");
        screenHeight_ = Ngin::envget<int>("screen.height");

        // Generate framebuffer
        glGenFramebuffers(1, &aaFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, aaFBO_);

        // Generate texture to render to
        glGenTextures(1, &aaTexture_);
        glBindTexture(GL_TEXTURE_2D, aaTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth_, screenHeight_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aaTexture_, 0);

        // Create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &aaRBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, aaRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth_, screenHeight_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, aaRBO_);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Log::console("Post-processing framebuffer not complete!", 1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void render() override {
        screenWidth_ = Ngin::envget<int>("screen.width");
        screenHeight_ = Ngin::envget<int>("screen.height");
        
        // draw post
        glDisable(GL_DEPTH_TEST);
        Context::framebuffer(0);

        // Render Post
        Context::texture(getAATexture(), 0);
        shaderPost.setVec2("screenSize", glm::vec2(screenWidth_, screenHeight_));

        Drawer::prep("antialiasing");
        renderQuad();
    }
    void bind() override {
    }

    unsigned int getAABuffer() {
        return aaFBO_;
    }
    unsigned int getAATexture() {
        return aaTexture_;
    }

private:
    unsigned int aaFBO_, aaRBO_, aaTexture_;

    unsigned int screenWidth_, screenHeight_;

    ShaderData& shaderPost = Resources::getShaderData("antialiasing");

    unsigned int quadVAO_ = 0;
    unsigned int quadVBO_ = 0;
    void renderQuad()
    {
        if (quadVAO_ == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO_);
            glGenBuffers(1, &quadVBO_);
            glBindVertexArray(quadVAO_);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

};

#endif // POST_PASS_H
