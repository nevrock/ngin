#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nev/fileutils.h>
#include <nev/shader.h>
#include "camera/camera.h"
#include "light/light.h"
#include <nev/window.h>
#include <nev/resources.h>
#include <nev/scene.h>
#include <nev/constants.h>
#include <nev/sound_manager.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <thread>
#include <atomic>
#include <nev/game.h>

#include <nev/dict.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = nev::SCREEN_WIDTH;
const unsigned int SCR_HEIGHT = nev::SCREEN_HEIGHT;

Camera* camera;
Light* light;
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

// Initialize static members
ISoundEngine* SoundManager::engine = nullptr;
std::unordered_map<unsigned int, ISound*> SoundManager::sounds;
std::atomic<unsigned int> SoundManager::nextSoundId{1};  // Start IDs from 1

int main()
{
    Window window(SCR_WIDTH, SCR_HEIGHT, "nev_v1");
    GLFWwindow* win = Window::getGLFWwindow();

    Game::setState("loading");
    Game::setState("start");


    SoundManager sound;
    //sound.play(std::string("sleep_song"));
    //SoundManager::play(std::string("sleep_song"));

    Scene scene("scene_start");

    scene.initScene();
    
    //scene.initScene([&scene]() {  // Capture scene by reference
    //    auto camera = Camera::getMainCamera();
    //    auto light = Light::getMainLight();

    //    if (camera && light) {  // Ensure these objects are valid
    //        scene.initDepth();
    //        scene.launch();
    //    } else {
    //        std::cerr << "Failed to initialize camera or light" << std::endl;
    //    }
    //});
    scene.build();

    std::atomic<bool> running(true);
    std::thread physicsThread([&]() {
        float lastFramePhysics = static_cast<float>(glfwGetTime());  // Initialize lastFramePhysics here
        while (running) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTimePhysics = currentFrame - lastFramePhysics;

            if (deltaTimePhysics > fixedPhysicsStep) {
                lastFramePhysics = currentFrame;
                scene.updatePhysics(deltaTimePhysics);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Simulate the passage of time between frames
        }
    });

    camera = Camera::getMainCamera();
    light = Light::getMainLight();


    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetScrollCallback(win, scroll_callback);

    // configure depth map FBO
    // -----------------------
    window.setupDepthMap();
    window.setupCubeMap();

    // shader configuration
    // --------------------
    scene.initDepth();
    scene.launch();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(win))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::string& state = Game::getState();

        // input
        // -----
        processInput(win);

        window.clear();

        if (state != "loading") {

            scene.update();
            scene.updateAnimation(deltaTime);
            
            // 1. render depth of scene to texture (from light's perspective)
            // --------------------------------------------------------------
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            scene.preRender(0);
            window.updateDepthMap();
            scene.render(0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // render scene from light's point of view
            scene.preRender(1);
            window.updateCubeMap();
            scene.render(1);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // reset viewport
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 2. render scene as normal using the generated depth/shadow map  
            // --------------------------------------------------------------

            for (int i = nev::RENDER_LAYER_THRESHOLD_SHADOWS; i < nev::RENDER_LAYER_THRESHOLD_UI; i++) {
                scene.preRender(i);
                window.bindDepthMap();
                window.bindCubeMap();
                scene.render(i);
            }

            // 3. render ui
            // --------------------------------------------------------------
            glDisable(GL_DEPTH_TEST);
            
            int idx = nev::RENDER_LAYER_UI;
            scene.preRender(idx);
            scene.render(idx);

            std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Simulate the passage of time between frames
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    scene.clear();

    running = false;

    physicsThread.join();  

    glfwTerminate();

    return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(RIGHT, deltaTime);
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

    camera->processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera->processMouseScroll(static_cast<float>(yoffset));
}
