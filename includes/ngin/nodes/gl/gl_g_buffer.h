#ifndef GL_G_BUFFER_H
#define GL_G_BUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/node/node.h>
#include <ngin/game.h>

class GlGBuffer : public Node {
public:
    explicit GlGBuffer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary)
    {
    }
    ~GlGBuffer() override = default;

    void update(std::string& pass) override {
        Node::update(pass); 
    }
    void start(std::string& pass) override {
        Node::start(pass);

        int screenWidth = Game::env<int>("screen.width");
        int screenHeight = Game::env<int>("screen.height");

        glGenFramebuffers(1, &gBuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_);
        // position color buffer
        glGenTextures(1, &gPosition_);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition_, 0);
        // normal color buffer
        glGenTextures(1, &gNormal_);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal_, 0);
        // color + specular color buffer
        glGenTextures(1, &gAlbedoSpec_);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec_, 0);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &rboDepth_);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth_);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
        // set game env variables
        Game::envset("g_buffer", gBuffer_);
    }   

protected:
    unsigned int gBuffer_;
    unsigned int gPosition_, gNormal_, gAlbedoSpec_;
    unsigned int rboDepth_;

};

#endif