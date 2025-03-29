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
#include <ngin/gl.h>

#include <ngin/render/shadow_pass.h> 
#include <ngin/render/geometry_pass.h> 
#include <ngin/render/ssao_pass.h> 
#include <ngin/render/env_pass.h> 
#include <ngin/render/post_pass.h> 

class Renderer {

public:
    Renderer() {
        Gl::createWindow("ngin", 
            Game::envget<int>("screen.width"), 
            Game::envget<int>("screen.height"),
            framebufferSizeCallback,
            mouseCallback,
            scrollCallback);
    }
    void setup() {
        shadowPass_ = new ShadowPass(1); // Provide the required argument to ShadowPass constructor
        ssaoPass_ = new SsaoPass(3); // Provide the required argument to SsaoPass constructor
        geomPass_ = new GeometryPass(2); // Provide the required argument to GeometryPass constructor
        geomPass_->linkSsaoColorBufferBlur(ssaoPass_->getSsaoColorBufferBlur());
        
        shadowPass_->setup();
        ssaoPass_->setup();
        geomPass_->setup();
    }
    void render() {
        if (shadowPass_) {
            shadowPass_->render();
        }
    }

private:
    ShadowPass* shadowPass_; // Updated type to ShadowPass*
    GeometryPass* geomPass_;
    SsaoPass* ssaoPass_;

    static inline float lastX_;
    static inline float lastY_;
    static inline bool firstMouse_;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Game::envset("screen.width", width);
        Game::envset("screen.height", height);
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        // Handle mouse movement
        std::cout << "Mouse moved to: " << xpos << ", " << ypos << std::endl;

        float xPosFloat = static_cast<float>(xpos);
        float yPosFloat = static_cast<float>(ypos);

        Game::envset("mouse.x", xPosFloat);
        Game::envset("mouse.y", yPosFloat);

        if (firstMouse_) {
            lastX_ = xPosFloat;
            lastY_ = yPosFloat;
            firstMouse_ = false;
        }

        float xoffset = lastX_ - xPosFloat;
        float yoffset = lastY_ - yPosFloat; // reversed since y-coordinates go from bottom to top

        lastX_ = xPosFloat;
        lastY_ = yPosFloat;

        Game::envset("mouse.offsetX", xoffset);
        Game::envset("mouse.offsetY", yoffset);
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        // Handle scroll input
        std::cout << "Scrolled: " << xoffset << ", " << yoffset << std::endl;

        Game::envset("scroll_y", static_cast<float>(yoffset));
    }

};

#endif // RENDERER_H