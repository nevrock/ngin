#ifndef DEFERRED_PASS_H
#define DEFERRED_PASS_H

#include <iostream>
#include <functional> // For std::function

#include <ngin/render/render_pass.h>
#include <ngin/lighter.h>

class DeferredPass : public RenderPass {
public:
    DeferredPass(unsigned int id) : RenderPass(id) {}

    void setup() override {

    }
    void render() override {
        // PASS :: ssao deferred
        // Set framebuffer for SSAO deferred rendering
        Context::framebuffer(postFBO_);
        Context::clear(false);

        bind();

        // Update deferred lighting and draw SSAO deferred
        Lighter::updateDeferred("ssao_deferred");
        Drawer::prep("ssao_deferred");

        renderQuad();
    }
    void bind() override {
        if (geomBind_) geomBind_();
        if (shadowBind_) shadowBind_();
    }

    void linkBinding(std::function<void()> geomBind, std::function<void()> shadowBind) {
        geomBind_ = geomBind;
        shadowBind_ = shadowBind;
    }
    void linkPostBuffer(unsigned int postFBO) {
        postFBO_ = postFBO;
    }

private:
    unsigned int quadVAO_ = 0;
    unsigned int quadVBO_ = 0;
    std::function<void()> geomBind_;   // Function to bind geometry
    std::function<void()> shadowBind_; // Function to bind shadow

    unsigned int postFBO_;
    
    
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

#endif // DEFERRED_PASS_H
