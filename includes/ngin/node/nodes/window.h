#ifndef WINDOW_NODE_H
#define WINDOW_NODE_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/log.h>
#include <ngin/game.h>

#include <ngin/node/node.h>

class nWindow : public Node {
public:
    nWindow(unsigned int id, const Lex& lex) : window_(nullptr) {
        setId(id);
        loadFromLex(lex);
        // Initialize WindowNode-specific properties if needed
        Log::console("nWindow created with ID: " + std::to_string(id), 1);
    }
    void loadFromLex(const Lex& lex) override {
        Node::loadFromLex(lex);

        std::string title = lex.getC<std::string>("title", "NGIN");

        // Load additional properties here
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

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
        context_ = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);
        if (context_ == NULL)
        {
            Log::console("Failed to create GLFW window", 1);
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(context_);
    

        // tell GLFW to capture our mouse
        glfwSetInputMode(context_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
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

        Game::envset("screen.context", context_);
    }


    void setup() override {
        // Add WindowNode-specific setup logic here
        Log::console("Setting up nWindow with ID: " + std::to_string(getId()), 1);
    }
    void execute() override {
        Log::console("Executing nWindow with ID: " + std::to_string(getId()), 1);
        // Add WindowNode-specific execution logic here

        updateTime();
        processInput();

        if (!glfwWindowShouldClose(context_)) {
            Game::setRunning(false);
        } else {
            glfwSwapBuffers(context_);
            glfwPollEvents();
        }
    }
    void end() override {
        // Add WindowNode-specific end logic here
        glfwDestroyWindow(context_);
        glfwTerminate();
    }

private:
    GLFWwindow* context_; 
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;
    bool firstMouse_ = true;
    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    void updateTime() {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime_ = currentTime - lastFrame_;
        lastFrame_ = currentTime;

        Game::envset<float>("time.current", currentTime);
        Game::envset<float>("time.delta", deltaTime);

        Game::envset<float>("mouse.offsetX", MathUtils::lerp(Game::envget<float>("mouse.offsetX"), 0.0f, deltaTime * 5.0f));
        Game::envset<float>("mouse.offsetY", MathUtils::lerp(Game::envget<float>("mouse.offsetY"), 0.0f, deltaTime * 5.0f));
    }
    void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Game::envset("screen.width", width);
        Game::envset("screen.height", height);
    }
    void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        Game::envset("mouse.x", xpos);
        Game::envset("mouse.y", ypos);

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

        Game::envset("mouse.offsetX", xoffset);
        Game::envset("mouse.offsetY", yoffset);
    }
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Game::envset("scroll_y", static_cast<float>(yoffset));
    }
    void processInput() {
        if (glfwGetKey(mainContext, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mainContext, true);

        float axesX = Game::envget<float>("axesX");
        float axesY = Game::envget<float>("axesY");

        bool isY = false;
        bool isX = false;

        if (glfwGetKey(mainContext, GLFW_KEY_W) == GLFW_PRESS) {
            axesY = MathUtils::lerp(axesY, 1.0, deltaTime * 5.0f);
            isY = true;
        }
        if (glfwGetKey(mainContext, GLFW_KEY_S) == GLFW_PRESS) {
            axesY = MathUtils::lerp(axesY, -1.0, deltaTime * 5.0f);
            isY = true;
        }
        if (!isY) {
            axesY = MathUtils::lerp(axesY, 0.0, deltaTime * 5.0f);
        }

        if (glfwGetKey(mainContext, GLFW_KEY_A) == GLFW_PRESS) {
            axesX = MathUtils::lerp(axesX, -1.0, deltaTime * 5.0f);
            isX = true;
        }
        if (glfwGetKey(mainContext, GLFW_KEY_D) == GLFW_PRESS) {
            axesX = MathUtils::lerp(axesX, 1.0, deltaTime * 5.0f);
            isX = true;
        }
        if (!isX) {
            axesX = MathUtils::lerp(axesX, 0.0, deltaTime * 5.0f);
        }

        Game::envset<float>("axesX", axesX);
        Game::envset<float>("axesY", axesY);
    }
};

#endif // WINDOW_NODE_H
