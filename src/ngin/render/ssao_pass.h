#ifndef SSAO_PASS_H
#define SSAO_PASS_H

#include <ngin/render/render_pass.h>
#include <iostream>

class SsaoPass : public RenderPass {
public:
    SsaoPass(unsigned int id) : RenderPass(id) {}

    void setup() override {
        std::cout << "Setting up ssao pass with ID: " << getId() << std::endl;

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
            scale = lerp(0.1f, 1.0f, scale * scale);
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
    void render() override {
        std::cout << "Rendering shadows for pass ID: " << getId() << std::endl;

    }
    void bind() override {
        
    }

    unsigned int getSsaoColorBufferBlur() {
        return ssaoColorBufferBlur_;
    }

private:
    unsigned int ssaoFBO_, ssaoBlurFBO_;
    unsigned int noiseTexture_; 
    unsigned int ssaoColorBuffer_, ssaoColorBufferBlur_;

    std::vector<glm::vec3> ssaoKernel_;

    float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

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

#endif // SSAO_PASS_H
