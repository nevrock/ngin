#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

#include <iostream>

#include <ngin/render/render_pass.h>

class ShadowPass : public RenderPass {
public:
    ShadowPass(unsigned int id) : RenderPass(id) {}

    void setup() override {
        std::cout << "Setting up shadows pass with ID: " << getId() << std::endl;

        shadowWidth_ = Ngin::envget<int>("shadow.width");
        shadowHeight_ = Ngin::envget<int>("shadow.height");

        glGenFramebuffers(1, &shadowMapFBO_);
        glGenTextures(1, &shadowMap_);
        glBindTexture(GL_TEXTURE_2D, shadowMap_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth_, shadowHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void render() override {

        // Clear buffer and enable depth testing and face culling
        Context::clear(true);
        Context::depth(true);
        Context::cull(true);   

        // PASS :: shadows
        // Set viewport and framebuffer for shadow rendering
        Context::viewport(shadowWidth_, shadowHeight_);
        Context::framebuffer(shadowMapFBO_);
        Context::clear(true);

        // Prepare and draw shadow map
        Drawer::prep("shadowmap");
        Drawer::draw("shadowmap");
        
    }
    void bind() override {
        Context::texture(shadowMap_, 0);
    }

    unsigned int getShadowBuffer() {
        return shadowMapFBO_;
    }

private:
    unsigned int shadowMapFBO_, shadowMap_;
    unsigned int shadowWidth_, shadowHeight_;

};

#endif // SHADOW_PASS_H
