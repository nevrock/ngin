#ifndef GL_H
#define GL_H

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/log.h>
#include <ngin/game.h>
#include <ngin/utils/mathutils.h>

#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <random>

class Gl {
    
public:

    static void createWindow(const std::string& title, 
                        int screenWidth, 
                        int screenHeight,
                        void (*framebufferSizeCallback)(GLFWwindow*, int, int),
                        void (*mouseCallback)(GLFWwindow*, double, double),
                        void (*scrollCallback)(GLFWwindow*, double, double)) {

        glfwInit();
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        context_ = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);
        if (context_ == NULL)
        {
            Log::console("failed to create GLFW window", 1);
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(context_);
    
        glfwSetInputMode(context_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            Log::console("Failed to initialize GLAD", 1);
            return;
        }

        glfwSetFramebufferSizeCallback(context_, framebufferSizeCallback);
        glfwSetCursorPosCallback(context_, mouseCallback);
        glfwSetScrollCallback(context_, scrollCallback);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glfwSwapInterval(1);
    }
    static bool shouldCloseWindow() {
        return glfwWindowShouldClose(context_);
    }
    static float getTime() {
        float currentTime = static_cast<float>(glfwGetTime());
        return currentTime;
    }
    static bool getKey(std::string& key) {
        if (key == "a") {
            return glfwGetKey(context_, GLFW_KEY_A) == GLFW_PRESS;
        } else if (key == "d") {
            return glfwGetKey(context_, GLFW_KEY_D) == GLFW_PRESS;
        } else if (key == "w") {
            return glfwGetKey(context_, GLFW_KEY_W) == GLFW_PRESS;
        } else if (key == "s") {
            return glfwGetKey(context_, GLFW_KEY_S) == GLFW_PRESS;
        } else if (key == "esc") {
            return glfwGetKey(context_, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        }
    }
    
    static void setTexture(unsigned int setTexture, int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, setTexture);
    }
    static void setFramebuffer(unsigned int fbo, bool isClear = false) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (isClear) {
            clearBuffer(true);
        }
    }
    static void setCull(bool isCull, bool isFront=false) {
        if (isCull) {
            glEnable(GL_CULL_FACE);
            if (isFront) {
                glCullFace(GL_FRONT);
            } else {
                glCullFace(GL_BACK);
            }
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
    static void setDepth(bool isDepthTest) {
        if (isDepthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
    }
    static void setBlendMode(bool isTransparent) {
        if (isTransparent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    static void setViewport(int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void displayAndPoll() {
        display();
        poll();
    }
    static void display() {
        glfwSwapBuffers(context_);
    }
    static void poll() {
        glfwPollEvents();
    }
    static void clearBuffer(bool isClearColor=false) {
        if (isClearColor)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    static void terminateWindow() {
        if (context_) {
            glfwDestroyWindow(context_);
        }
        glfwTerminate();
    }

private:
    static inline GLFWwindow* context_;
};

#endif // GL_H