#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/log.h>
#include <ngin/ngin.h>
#include <ngin/utils/mathutils.h>

#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <random>
#include <stb_image.h> // Include stb_image for image loading

class Context {
public:
    static inline GLFWwindow* mainGl; // Now a static member

    static inline float deltaTime = 0.0f;
    static inline float lastFrame = 0.0f;

    static void create(const std::string& title) {

        int screenWidth = Ngin::envget<int>("screen.width");
        int screenHeight = Ngin::envget<int>("screen.height");

        lastX_ = (float)screenWidth / 2.0;
        lastY_ = (float)screenHeight / 2.0;

        // glfw: initialize and configure
        // ------------------------------
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        // glfw window creation
        // --------------------
        mainGl = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);
        if (mainGl == NULL)
        {
            Log::console("failed to create GLFW window", 1);
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(mainGl);
    

        // tell GLFW to capture our mouse
        glfwSetInputMode(mainGl, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            Log::console("Failed to initialize GLAD", 1);
            return;
        }

        glfwSetFramebufferSizeCallback(mainGl, framebufferSizeCallback);
        glfwSetCursorPosCallback(mainGl, mouseCallback);
        glfwSetScrollCallback(mainGl, scrollCallback);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glfwSwapInterval(1);
    }
    
    static void framebuffer(unsigned int fbo, bool isClear = false) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (isClear) {
            clear(true);
        }
    }
    static void texture(unsigned int texture, int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    static void cull(bool isCull, bool isFront=false) {
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
    static void depth(bool isDepth) {
        if (isDepth) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
    }
    static void blend(bool isTransparent) {
        if (isTransparent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    static void viewport(int width, int height) {
        glViewport(0, 0, width, height);
    }

    

    static bool shouldClose() {
        return glfwWindowShouldClose(mainGl);
    }
    static void displayAndPoll() {
        display();
        pollEvents();
    }
    static void display() {
        glfwSwapBuffers(mainGl);
    }
    static void pollEvents() {
        glfwPollEvents();
    }
    static void updateTime() {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        Ngin::envset<float>("time.current", currentTime);
        Ngin::envset<float>("time.delta", deltaTime);
        
        if (timer_ > 0.1f) {
            Ngin::envset<float>("mouse.offsetX", MathUtils::lerp(Ngin::envget<float>("mouse.offsetX"), 0.0f, deltaTime * 5.0f));
            Ngin::envset<float>("mouse.offsetY", MathUtils::lerp(Ngin::envget<float>("mouse.offsetY"), 0.0f, deltaTime * 5.0f));    
        }

        timer_ += deltaTime;
    }
    static void clear(bool isClearColor=false) {
        if (isClearColor)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    static void terminate() {
        glfwTerminate();
    }

    ~Context() {
        glfwDestroyWindow(mainGl);
        glfwTerminate();
        mainGl = nullptr; // Reset static member
    }

    static void processInput() {
        if (glfwGetKey(mainGl, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mainGl, true);

        float axesX = Ngin::envget<float>("axesX");
        float axesY = Ngin::envget<float>("axesY");

        bool isY = false;
        bool isX = false;

        if (glfwGetKey(mainGl, GLFW_KEY_W) == GLFW_PRESS) {
            axesY = MathUtils::lerp(axesY, 1.0, deltaTime * 5.0f);
            isY = true;
        }
        if (glfwGetKey(mainGl, GLFW_KEY_S) == GLFW_PRESS) {
            axesY = MathUtils::lerp(axesY, -1.0, deltaTime * 5.0f);
            isY = true;
        }
        if (!isY) {
            axesY = MathUtils::lerp(axesY, 0.0, deltaTime * 5.0f);
        }

        if (glfwGetKey(mainGl, GLFW_KEY_A) == GLFW_PRESS) {
            axesX = MathUtils::lerp(axesX, -1.0, deltaTime * 5.0f);
            isX = true;
        }
        if (glfwGetKey(mainGl, GLFW_KEY_D) == GLFW_PRESS) {
            axesX = MathUtils::lerp(axesX, 1.0, deltaTime * 5.0f);
            isX = true;
        }
        if (!isX) {
            axesX = MathUtils::lerp(axesX, 0.0, deltaTime * 5.0f);
        }

        Ngin::envset<float>("axesX", axesX);
        Ngin::envset<float>("axesY", axesY);
    }

    static void setCursorEnabled(bool enabled) {
        if (enabled) {
            glfwSetInputMode(mainGl, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(mainGl, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    
    static void setCustomCursor(const std::string& imagePath, int hotspotX, int hotspotY) {
        GLFWimage image;
        // Load the image (implementation depends on your image loading library)
        // Example: Load image data into `image.pixels`, and set `image.width` and `image.height`
        if (!loadImage(imagePath, image)) {
            Log::console("Failed to load cursor image: " + imagePath, 1);
            return;
        }

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
        if (!cursor) {
            Log::console("Failed to create custom cursor", 1);
            return;
        }

        glfwSetCursor(mainGl, cursor);

        // Free image data if necessary (depends on your image loading library)
        freeImage(image);
    }

private:
    static inline float lastX_ = 0.0f;
    static inline float lastY_ = 0.0f;
    static inline bool firstMouse_ = true;

    static inline float timer_ = 0.0f;

    static bool loadImage(const std::string& path, GLFWimage& image) {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &image.width, &image.height, &channels, STBI_rgb_alpha);
        if (!data) {
            Log::console("Failed to load image: " + path, 1);
            return false;
        }

        image.pixels = data; // Assign loaded image data to GLFWimage
        return true;
    }

    static void freeImage(GLFWimage& image) {
        if (image.pixels) {
            stbi_image_free(image.pixels); // Free the image data
            image.pixels = nullptr;
        }
    }

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Ngin::envset("screen.width", width);
        Ngin::envset("screen.height", height);
    }
    static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
        // Implement mouse position handling logic here
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        Ngin::envset("mouse.x", xpos);
        Ngin::envset("mouse.y", ypos);


        if (firstMouse_)
        {
            lastX_ = xpos;
            lastY_ = ypos;
            firstMouse_ = false;
        }

        float xoffset = lastX_ - xpos;
        float yoffset = lastY_ - ypos; // reversed since y-coordinates go from bottom to top

        lastX_ = xpos;
        lastY_ = ypos;

        Ngin::envset("mouse.offsetX", xoffset);
        Ngin::envset("mouse.offsetY", yoffset);

        timer_ = 0.0f;

        //Log::console("mouse_x: " + std::to_string(xoffset) + ", mouse_y: " + std::to_string(yoffset));
    }
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        // Implement scroll handling logic here

        Ngin::envset("scroll_y", static_cast<float>(yoffset));
    }
};

#endif // CONTEXT_H