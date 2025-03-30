#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H

#include <ngin/render/render_pass.h>
#include <iostream>

class GeometryPass : public RenderPass {
public:
    GeometryPass(unsigned int id) : RenderPass(id) {}

    void setup() override {
        std::cout << "Setting up geom pass with ID: " << getId() << std::endl;

        screenWidth_ = Game::envget<int>("screen.width");
        screenHeight_ = Game::envget<int>("screen.height");
        // configure g-buffer framebuffer
        glGenFramebuffers(1, &forwardFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
        // position color buffer
        glGenTextures(1, &gPosition_);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth_, screenHeight_, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition_, 0);
        // normal color buffer
        glGenTextures(1, &gNormal_);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth_, screenHeight_, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal_, 0);
        // color + specular color buffer
        glGenTextures(1, &gAlbedoSpec_);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth_, screenHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec_, 0);
        // world position color buffer
        glGenTextures(1, &gPositionWorld_);
        glBindTexture(GL_TEXTURE_2D, gPositionWorld_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth_, screenHeight_, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPositionWorld_, 0);
        // world normal color buffer
        glGenTextures(1, &gNormalWorld_);
        glBindTexture(GL_TEXTURE_2D, gNormalWorld_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth_, screenHeight_, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gNormalWorld_, 0);
        // depth texture
        glGenTextures(1, &gDepth_);
        glBindTexture(GL_TEXTURE_2D, gDepth_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth_, screenHeight_, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gDepth_, 0);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(6, attachments);
        
        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &forwardRBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, forwardRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth_, screenHeight_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, forwardRBO_);
        
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("Framebuffer not complete!", 1);
            
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void render() override {
        // Reset viewport and framebuffer
        Context::viewport(screenWidth_, screenHeight_);
        Context::framebuffer(0);
        Context::clear(true);

        // PASS :: forward
        // Set framebuffer for forward rendering
        Context::framebuffer(forwardFBO_);
        Context::clear(false);

        // Prepare and draw SSAO G-buffer
        Drawer::prep("gbuffer");
        Drawer::draw("gbuffer");
    }
    void linkSsaoColorBufferBlur(unsigned int ssaoColorBufferBlur) {
        ssaoColorBufferBlur_ = ssaoColorBufferBlur;
    }
    void copyDepthBuffer(unsigned int drawBuffer = 0) {
        int screenWidth_ = Game::envget<int>("screen.width");
        int screenHeight_ = Game::envget<int>("screen.height");

        glBindFramebuffer(GL_READ_FRAMEBUFFER, forwardFBO_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawBuffer); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, screenWidth_, screenHeight_, 0, 0, screenWidth_, screenHeight_, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, drawBuffer);
    }
    void bind() override {
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gPosition_);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gNormal_);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);

        glActiveTexture(GL_TEXTURE4); 
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gPositionWorld_);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, gNormalWorld_);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, gDepth_);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Game::envget<int>("envCubemap"));
    }

    unsigned int getForwardBuffer() {
        return forwardFBO_;
    }
    unsigned int getPositionTexture() {
        return gPosition_;
    }
    unsigned int getNormalTexture() {
        return gNormal_;
    }
    unsigned int getAlbedoSpecTexture() {
        return gAlbedoSpec_;
    }
    unsigned int getPositionWorldTexture() {
        return gPositionWorld_;
    }
    unsigned int getNormalWorldTexture() {
        return gNormalWorld_;
    }
    unsigned int getDepthTexture() {
        return gDepth_;
    }
    

private:
    unsigned int forwardFBO_, gDepth_, gPosition_, gPositionWorld_, gNormalWorld_, gNormal_, gAlbedoSpec_, forwardRBO_;
    unsigned int ssaoColorBufferBlur_;

    int screenWidth_, screenHeight_;
};

#endif // GEOMETRY_PASS_H
