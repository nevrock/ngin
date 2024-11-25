#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ngin/utils/fileutils.h>
#include <ngin/gl/window.h>
#include <ngin/constants.h>
#include <ngin/collections/nevf.h>
#include <ngin/game.h>
#include <ngin/resources.h>

#include <ngin/node/node_graph.h>
#include <ngin/node/types/pass.h>
#include <ngin/node/graph_state.h>
#include <ngin/scene.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <thread>
#include <atomic>
#include <memory>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = ngin::SCREEN_WIDTH;
const unsigned int SCR_HEIGHT = ngin::SCREEN_HEIGHT;

// camera
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float deltaTimePhysics = 0.0f;
float lastFramePhysics = 0.0f;

float fixedPhysicsStep = 0.015f;

unsigned int screenWidth, screenHeight;

GLFWwindow* Window::window = nullptr; // Initialize static member
Window* Window::mainWindow = nullptr; // Initialize static member

int main()
{
    Window window(SCR_WIDTH, SCR_HEIGHT, "nev_v1");
    GLFWwindow* win = Window::getGLFWwindow();

    Game::setState("loading");
    Game::setState("start");

    Nevf n = Resources::loadNevf("game");
    Scene scene;
    scene.load(n.getC<std::string>("start_scene", "scenes/start").c_str());
    /*
    NodeGraph nodeGraph(n.getC<std::string>("start_scene", "scenes/start"));
    std::shared_ptr<Pass> passNode = nodeGraph.getNodeByType<Pass>();
    if (passNode != nullptr) {
        std::cout << "found pass node!" << std::endl;

        GraphState graph(passNode);
        graph.cook();
    }
    */

    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetScrollCallback(win, scroll_callback);

    window.setupDepthMap();
    window.setupCubeMap();

    std::atomic<bool> running(true);

    while (!glfwWindowShouldClose(win))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::string& state = Game::getState();

        // input
        // -----
        processInput(win);

        window.clear();

        if (state != "loading") {
            
            // 1. render depth of scene to texture (from light's perspective)
            // --------------------------------------------------------------
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            window.updateDepthMap();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // render scene from light's point of view
            window.updateCubeMap();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // reset viewport
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 2. render scene as normal using the generated depth/shadow map  
            // --------------------------------------------------------------

            for (int i = ngin::RENDER_LAYER_THRESHOLD_SHADOWS; i < ngin::RENDER_LAYER_THRESHOLD_UI; i++) {
                window.bindDepthMap();
                window.bindCubeMap();
            }

            // 3. render ui
            // --------------------------------------------------------------
            glDisable(GL_DEPTH_TEST);
            

            std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Simulate the passage of time between frames
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    running = false;

    glfwTerminate();

    return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}
