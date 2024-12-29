#ifndef GL_SHADOWS_POINT_H
#define GL_SHADOWS_POINT_H

#include <GLFW/glfw3.h>
#include <ngin/node/node.h>

class GlShadowsPoint : public Node {
public:
    GlShadowsPoint(const std::string& name, Nevf& data) : Node(name, data) {
    }

    // Override the execute method
    void update(std::string& pass) override {
        Node::update(pass); // retrieve inputs

        if (pass.find("shadows") == std::string::npos) {
            updateCubeMap();
        } else {
            bindCubeMap();
        }
    }

    void setup() override {
        Node::setup();

        setupCubeMap();
    }


private:
    unsigned int cubeMapFBO, cubeMap;
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    void setupCubeMap() {
        glGenFramebuffers(1, &cubeMapFBO);
        glGenTextures(1, &cubeMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, cubeMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void updateCubeMap() {
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, cubeMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void bindCubeMap() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    }
};

#endif // GL_SHADOWS_POINT_H