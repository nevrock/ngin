#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ngin/gl/window.h>
#include <ngin/collections/nevf.h>
#include <ngin/game.h>
#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <ngin/scene.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <thread>
#include <atomic>
#include <memory>


// --- STATIC DECLARATIONS --- //
Nevf Resources::shaderManifest_ = {}; 
Nevf Resources::meshManifest_ = {}; 
std::map<std::string, std::vector<IDrawer*>> Drawer::drawers_ = {}; 

GLFWwindow* Window::window = nullptr; // Initialize static member
Window* Window::mainWindow = nullptr; // Initialize static member

static int gameInit = (Game::init(), 0); 
static int resourcesInit = (Resources::init(), 0); 


// --- GLOBAL VARIABLES --- //
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// --- MAIN --- //
int main()
{
    // --- game setup --- //
    Game::setState("loading");
    Nevf n = Resources::loadNevf("game");
    Nevf env = n.getC<Nevf>("env", Nevf());
    Game::setEnv(std::make_shared<Nevf>(env));

    // --- scene setup --- /
    Scene scene;
    scene.load(n.getC<std::string>("start_scene", "scenes/start").c_str());

    // --- window setup --- //
    Window window(Game::env<int>("screen.width"), Game::env<int>("screen.height"), "ngin");

    // --- main loop --- //
    Game::setState("start");


    while (!window.shouldClose())
    {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        Game::envset<float>("time.current", currentTime);
        Game::envset<float>("time.delta", deltaTime);

        window.processInput();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        window.clear();

        scene.executePasses();

        window.displayAndPoll();
    }


    window.terminate();
    return 0;
}
