#ifndef SSAO_PASS_H
#define SSAO_PASS_H

#include <ngin/node/node.h>
#include <ngin/resources.h>
#include <ngin/log.h>
#include <iostream>

// Include glad header
#include <glad/glad.h>

class nSsao : public Node {
public:
    nSsao(unsigned int id, const Lex& lex) {
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
        Log::console("Setting up nSsao with ID: " + std::to_string(getId()), 1);

    }
    void execute() override {
        Log::console("Executing nSsao with ID: " + std::to_string(getId()), 1);

    }
private:
    unsigned int shadowMapFBO_, shadowMap_;
    unsigned int shadowWidth_ = 1024, shadowHeight_ = 1024;

    void setupSsao() {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");
        // also create framebuffer to hold SSAO processing stage 
        // -----------------------------------------------------
        glGenFramebuffers(1, &ssaoFBO_);  glGenFramebuffers(1, &ssaoBlurFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
        // SSAO color buffer
        glGenTextures(1, &ssaoColorBuffer_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("SSAO Framebuffer not complete!", 1);
        // and blur stage
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO_);
        glGenTextures(1, &ssaoColorBufferBlur_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("SSAO Blur Framebuffer not complete!", 1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // generate sample kernel
        // ----------------------
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;
        for (unsigned int i = 0; i < 64; ++i)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0f;

            // scale samples s.t. they're more aligned to center of kernel
            scale = ourLerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel_.push_back(sample);
        }

        // generate noise texture
        // ----------------------
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }

        ShaderData& shader = Resources::getShaderData("ssao");
        shader.use();
        for (unsigned int i = 0; i < 64; ++i)
                shader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);
        
        glGenTextures(1, &noiseTexture_);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
};

#endif
