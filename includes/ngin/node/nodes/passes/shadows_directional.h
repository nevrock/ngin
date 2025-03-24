#ifndef SHADOWS_DIRECTIONAL_PASS_H
#define SHADOWS_DIRECTIONAL_PASS_H

#include <ngin/node/node.h>
#include <ngin/resources.h>
#include <ngin/log.h>
#include <iostream>

// Include glad header
#include <glad/glad.h>

class nShadowsDirectional : public Node {
public:
    nShadowsDirectional(unsigned int id, const Lex& lex) {
        setId(id);
        loadFromLex(lex);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);
        shadowWidth_ = lex.getC<unsigned int>("width", 1024);
        shadowHeight_ = lex.getC<unsigned int>("height", 1024);

        setupShadowMap();
    }
    void setup() override {
        Log::console("Setting up nShadowsDirectional with ID: " + std::to_string(getId()), 1);

    }
    void execute() override {
        Log::console("Executing nShadowsDirectional with ID: " + std::to_string(getId()), 1);

    }
private:
    unsigned int shadowMapFBO_, shadowMap_;
    unsigned int shadowWidth_ = 1024, shadowHeight_ = 1024;

    void setupShadowMap() {
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
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

#endif
