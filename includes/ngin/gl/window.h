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
    Window(int windowWidth, int windowHeight, const std::string& title) {

        int screenWidth = Game::env<int>("screen.width");
        int screenHeight = Game::env<int>("screen.height");

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        // glfw window creation
        // --------------------
        GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        mainWindow = this;
    }


    bool shouldClose() const {
        return glfwWindowShouldClose(window);
    }
    void displayAndPoll() const {
        display();
        pollEvents();
    }
    void display() const {
        glfwSwapBuffers(window);
    }
    void pollEvents() const {
        glfwPollEvents();
    }

    void clear() {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void terminate() {
        glfwTerminate();
    }
    ~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
        window = nullptr; // Reset static member
    }

    static GLFWwindow* getGLFWwindow() { // Implement this method
        return window;
    }
    static Window* getMainWindow() {
        return mainWindow;
    }

    void processInput() {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
    

private:
    static GLFWwindow* window; // Now a static member
    static Window* mainWindow;    
    

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        // Implement mouse position handling logic here
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        // Implement scroll handling logic here
    }
};

#endif // WINDOW_H
