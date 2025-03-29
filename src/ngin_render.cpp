#include <string>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <ngin/utils/mathutils.h>
#include <ngin/log.h>
#include <ngin/lex.h>
#include <ngin/game.h>
#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <ngin/lighter.h>

#include <ngin/render/context.h>

#include <ngin/render/shadow_pass.h> 
#include <ngin/render/geometry_pass.h> 
#include <ngin/render/ssao_pass.h> 
#include <ngin/render/env_pass.h> 
#include <ngin/render/skybox_pass.h> 
#include <ngin/render/post_pass.h> 
#include <ngin/render/deferred_pass.h> 
#include <ngin/render/translucent_pass.h> 
#include <ngin/render/gui_pass.h> 


// --- STATIC FUNCTIONS --- //
static int gameInit = (Game::init(), 0);
static int resourcesInit = (Resources::init(), 0);
static int lighterInit = (Lighter::init(), 0);

float deltaTime_, lastFrame_;
float lastX_, lastY_;
std::string title = "ngin";
bool firstMouse_;


int main()
{
    // Create a context with the title "ngin"
    Context::create("ngin");

    // Initialize game and drawer components
    Game::start();

    // Initialize the render passes
    ShadowPass* shadowPass = new ShadowPass(1);
    shadowPass->setup();

    GeometryPass* geomPass = new GeometryPass(2);
    geomPass->setup();

    SsaoPass* ssaoPass = new SsaoPass(3);
    ssaoPass->setup();

    DeferredPass* deferredPass = new DeferredPass(4);
    deferredPass->setup();

    EnvPass* envPass = new EnvPass(5);
    envPass->setup();

    SkyboxPass* skyboxPass = new SkyboxPass(6);
    skyboxPass->setup();

    TranslucentPass* translucentPass = new TranslucentPass(7);
    translucentPass->setup();

    PostPass* postPass = new PostPass(8);
    postPass->setup();

    GuiPass* guiPass = new GuiPass(9);
    guiPass->setup();


    // Link passes together
    geomPass->linkSsaoColorBufferBlur(ssaoPass->getSsaoColorBufferBlur());
    ssaoPass->linkGBuffer(geomPass->getPositionTexture(), geomPass->getNormalTexture());
    
    deferredPass->linkBinding(
        [&]() { geomPass->bind(); },
        [&]() { shadowPass->bind(); }
    );
    deferredPass->linkPostBuffer(postPass->getPostBuffer());

    translucentPass->linkBinding(
        [&]() { shadowPass->bind(); }
    );

    envPass->render();

    // Render loop
    // -----------
    while (!Context::shouldClose())
    {
        // Update game logic and transformations
        Game::updateLogic();
        Game::updateTransform(); 

        // Update context time and process input
        Context::updateTime();
        Context::processInput();

        // Shadows pass
        shadowPass->render();
        
        // Geometry pass
        geomPass->render();

        // SSAO pass
        ssaoPass->render();
        
        // Deferred pass
        deferredPass->render();

        // Copy depth buffer and set depth function and face culling for skybox
        geomPass->copyDepthBuffer(postPass->getPostBuffer());

        // Env pass (skybox)
        skyboxPass->render();

        // Translucent pass
        translucentPass->render();

        // Post pass
        postPass->render();
        
        // Gui pass
        guiPass->render();  

        // Display the rendered frame and poll for events
        Context::displayAndPoll();

        // Sleep for a short duration (commented out)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Cleanup dynamically allocated passes
    delete shadowPass;
    delete geomPass;
    delete ssaoPass;
    delete envPass;
    delete postPass;
    delete deferredPass;
    delete translucentPass;
    delete guiPass;

    // Terminate the context and GLFW
    Context::terminate();

    return 0;
}