#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <iostream>
#include <random>
#include <vector> // Added for std::vector
#include <glad/glad.h>

#include <ngin/log.h>
#include <ngin/game.h>
#include <ngin/lex.h>

#include <ngin/render/context.h>

#include <ngin/render/shadow_pass.h> 
#include <ngin/render/geometry_pass.h> 
#include <ngin/render/ssao_pass.h> 
#include <ngin/render/env_pass.h> 
#include <ngin/render/post_pass.h> 

class Renderer {

public:

    static inline float deltaTime = 0.0f;
    static inline float lastFrame = 0.0f;

    Renderer() {
        Log::console("Renderer constructor called", 0);

        Context::create("ngin");
    }
    void setup() {
        shadowPass_ = new ShadowPass(1); // Provide the required argument to ShadowPass constructor
        shadowPass_->setup();

        geomPass_ = new GeometryPass(2);
        geomPass_->setup();

        ssaoPass_ = new SsaoPass(3);
        ssaoPass_->setup();

        envPass_ = new EnvPass(5);
        envPass_->setup();

        postPass_ = new PostPass(7);
        postPass_->setup();

        geomPass_->linkSsaoColorBufferBlur(ssaoPass.getSsaoColorBufferBlur());
    }
    void render() {
        shadowPass_->render();

        geomPass_->render();

        ssaoPass_->render();

        deferredPass_->render();  

        envPass_->render();

        translucentPass_->render();

        postPass_->render();

        guiPass_->render();
    }


private:
    ShadowPass* shadowPass_; // Updated type to ShadowPass*
    GeometryPass* geomPass_;
    SsaoPass* ssaoPass_;
    EnvPass* envPass_;
    PostPass* postPass_;
};

#endif // RENDERER_H