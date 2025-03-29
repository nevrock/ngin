#include <string>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <ngin/utils/mathutils.h>
#include <ngin/window.h>
#include <ngin/log.h>
#include <ngin/lex.h>
#include <ngin/game.h>
#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <ngin/lighter.h>

#include <ngin/render/renderer.h>


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
    // Create a window with the title "ngin"
    Window window("ngin");

    // Retrieve screen and shadow dimensions from the game environment
    int screenWidth = Game::envget<int>("screen.width");
    int screenHeight = Game::envget<int>("screen.height");
    int shadowWidth = Game::envget<int>("shadow.width");
    int shadowHeight = Game::envget<int>("shadow.height");

    // Initialize game and drawer components
    Game::start();

    // Setup the window
    window.setup();

    // Get the shaders we will need
    ShaderData& shaderSsao = Resources::getShaderData("ssao");
    ShaderData& shaderSsaoBlur = Resources::getShaderData("ssao_blur");
    ShaderData& shaderPost = Resources::getShaderData("post");

    // Render loop
    // -----------
    while (!window.shouldClose())
    {
        // Update game logic and transformations
        Game::updateLogic();
        Game::updateTransform();

        // Update window time and process input
        window.updateTime();
        window.processInput();

        // Clear buffer and enable depth testing and face culling
        window.clear(true);
        window.depth(true);
        window.cull(true);   

        // PASS :: shadows
        // Set viewport and framebuffer for shadow rendering
        window.viewport(shadowWidth, shadowHeight);
        window.framebuffer(window.getShadowBuffer());
        window.clear(true);

        // Prepare and draw shadow map
        Drawer::prep("shadowmap");
        Drawer::draw("shadowmap");
        
        // Reset viewport and framebuffer
        window.viewport(screenWidth, screenHeight);
        window.framebuffer(0);
        window.clear(true);

        // PASS :: forward
        // Set framebuffer for forward rendering
        window.framebuffer(window.getForwardBuffer());
        window.clear(false);

        // Prepare and draw SSAO G-buffer
        Drawer::prep("ssao_g_buffer");
        Drawer::draw("ssao_g_buffer");

        // Reset framebuffer
        window.framebuffer(0);
        window.clear(false);

        // PASS :: ssao
        // Set framebuffer for SSAO rendering
        window.framebuffer(window.getSsaoBuffer());
        window.clear(false);

        // Bind textures for SSAO
        window.texture(window.getPositionTexture(), 0);
        window.texture(window.getNormalTexture(), 1);
        window.texture(window.getNoiseTexture(), 2);

        // Prepare and draw SSAO
        Drawer::prep("ssao");
        Drawer::draw("ssao");

        // PASS :: ssao blur
        // Set framebuffer for SSAO blur rendering
        window.framebuffer(window.getSsaoBlurBuffer());
        window.clear(false);
        
        // Use SSAO blur shader and bind SSAO texture
        shaderSsaoBlur.use();
        window.texture(window.getSsaoTexture(), 0);

        // Prepare and draw SSAO blur
        Drawer::prep("ssao_blur");
        Drawer::draw("ssao_blur");
        
        // PASS :: ssao deferred
        // Set framebuffer for SSAO deferred rendering
        window.framebuffer(window.getPostBuffer());
        window.clear(false);

        // Bind SSAO G-buffer and shadow map
        window.bindSsaoGBuffer();
        window.bindShadowMap();

        // Update deferred lighting and draw SSAO deferred
        Lighter::updateDeferred("ssao_deferred");
        Drawer::prep("ssao_deferred");
        Drawer::draw("ssao_deferred");
        
        // PASS :: skybox
        // Copy depth buffer and set depth function and face culling for skybox
        window.copyDepthBuffer(window.getPostBuffer());
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);

        // Prepare and draw skybox
        Drawer::prep("skybox");
        Drawer::draw("skybox");

        // Reset face culling and depth function
        glCullFace(GL_BACK);   
        glDepthFunc(GL_LESS);

        // Render translucent objects
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Drawer::prep("translucent");
        window.bindShadowMap();
        Drawer::draw("translucent");



        // draw post
        glDisable(GL_DEPTH_TEST);
        window.framebuffer(0);

        // Render Post
        window.texture(window.getPostTexture(), 0);
        shaderPost.setVec2("screenSize", glm::vec2(screenWidth, screenHeight));

        Drawer::prep("post");
        Drawer::draw("post");


        // Render GUI
        //glDisable(GL_DEPTH_TEST);
        
        Drawer::prep("gui");
        Drawer::draw("gui");

        // Re-enable depth test after rendering GUI
        glEnable(GL_DEPTH_TEST);

        // Disable blending
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);

        // Display the rendered frame and poll for events
        window.displayAndPoll();

        // Sleep for a short duration (commented out)
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Terminate the window and GLFW
    window.terminate();
    glfwTerminate();
    return 0;
}