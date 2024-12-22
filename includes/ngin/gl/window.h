#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <ngin/log.h>

#include <ngin/game.h>

class Window {
public:
    static GLFWwindow* mainContext; // Now a static member
    static Window* mainWindow;    

    Window(const std::string& title) {

        int screenWidth = Game::env<int>("screen.width");
        int screenHeight = Game::env<int>("screen.height");

        lastX_ = (float)screenWidth / 2.0;
        lastY_ = (float)screenHeight / 2.0;

        // glfw: initialize and configure
        // ------------------------------
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        // glfw window creation
        // --------------------
        mainContext = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);
        if (mainContext == NULL)
        {
            std::cout << "failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(mainContext);
    

        // tell GLFW to capture our mouse
        glfwSetInputMode(mainContext, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }

        mainWindow = this;

        glfwSetFramebufferSizeCallback(mainContext, framebuffer_size_callback);
        glfwSetCursorPosCallback(mainContext, mouse_callback);
        glfwSetScrollCallback(mainContext, scroll_callback);

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_BLEND);  
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    }


    bool shouldClose() const {
        return glfwWindowShouldClose(mainContext);
    }
    void displayAndPoll() const {
        display();
        pollEvents();
    }
    void display() const {
        glfwSwapBuffers(mainContext);
    }
    void pollEvents() const {
        glfwPollEvents();
    }

    void clear(bool isClearColor=false) {
        if (isClearColor)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void terminate() {
        glfwTerminate();
    }
    ~Window() {
        glfwDestroyWindow(mainContext);
        glfwTerminate();
        mainContext = nullptr; // Reset static member
    }

    static GLFWwindow* getGLFWwindow() { // Implement this method
        return mainContext;
    }
    static Window* getMainWindow() {
        return mainWindow;
    }

    void processInput() {
        if (glfwGetKey(mainContext, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mainContext, true);
    }
    

private:
    static inline float lastX_ = 0.0f;
    static inline float lastY_ = 0.0f;
    static inline bool firstMouse_ = true;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Game::envset("screen.width", width);
        Game::envset("screen.height", height);
    }

    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
        // Implement mouse position handling logic here
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
        if (firstMouse_)
        {
            lastX_ = xpos;
            lastY_ = ypos;
            firstMouse_ = false;
        }

        float xoffset = xpos - lastX_;
        float yoffset = lastY_ - ypos; // reversed since y-coordinates go from bottom to top

        lastX_ = xpos;
        lastY_ = ypos;

        Game::envset("mouse_x", xoffset);
        Game::envset("mouse_y", yoffset);

        //Log::console("mouse_x: " + std::to_string(xoffset) + ", mouse_y: " + std::to_string(yoffset));
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        // Implement scroll handling logic here

        Game::envset("scroll_y", static_cast<float>(yoffset));
    }
};

#endif // WINDOW_H
