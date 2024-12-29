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

GLFWwindow* Window::mainContext = nullptr; // Initialize static member
Window* Window::mainWindow = nullptr; // Initialize static member

// Initialize static events
Event Window::onKeyPressW;
Event Window::onKeyPressS;
Event Window::onKeyPressA;
Event Window::onKeyPressD;


// --- STATIC FUNCTIONS --- //
static int gameInit = (Game::init(), 0); 
static int resourcesInit = (Resources::init(), 0); 



// --- MAIN --- //
int main()
{
    // --- game setup --- //
    Game::setState("loading");
    Nevf n = Resources::loadNevf("game");
    Nevf env = n.getC<Nevf>("env", Nevf());
    Game::setEnv(std::make_shared<Nevf>(env));

    // --- window setup --- //
    Window window("ngin");

    // --- scene setup --- /
    Scene scene;
    scene.load(n.getC<std::string>("start_scene", "scenes/start").c_str());

    // --- main loop --- //
    Game::setState("start");


    while (!window.shouldClose())
    {
        window.updateTime();

        window.processInput();
        window.clear(true);

        scene.executePasses();
        scene.cleanPasses();

        window.displayAndPoll();
    }

    window.terminate();

    Resources::terminate();
    Game::terminate();

    return 0;
}
