#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/log.h>
#include <ngin/game.h>
#include <ngin/utils/mathutils.h>

#include <ngin/resources.h>
#include <ngin/drawer.h>
#include <random>

class Window {
public:
    static inline GLFWwindow* mainContext; // Now a static member
    static inline Window* mainWindow;    

    static inline float deltaTime = 0.0f;
    static inline float lastFrame = 0.0f;

    Window(const std::string& title) {

        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        shadowHeight_= Game::envget<int>("shadow.height");
        shadowWidth_ = Game::envget<int>("shadow.width");

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
        mainContext = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);
        if (mainContext == NULL)
        {
            Log::console("failed to create GLFW window", 1);
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
            Log::console("Failed to initialize GLAD", 1);
            return;
        }

        mainWindow = this;

        glfwSetFramebufferSizeCallback(mainContext, framebuffer_size_callback);
        glfwSetCursorPosCallback(mainContext, mouse_callback);
        glfwSetScrollCallback(mainContext, scroll_callback);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glfwSwapInterval(1);
    }
    
    void setup() {
        setupShadowMap();

        setupGBuffer();
        setupSsao();
        setupSsaoCompute();

        setupEnvMap();
    }
    void framebuffer(unsigned int fbo, bool isClear = false) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (isClear) {
            clear(true);
        }
    }
    void texture(unsigned int texture, int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void cull(bool isCull, bool isFront=false) {
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
    void depth(bool isDepth) {
        if (isDepth) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
    }
    void blend(bool isTransparent) {
        if (isTransparent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    void viewport(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void setupEnvMap() {
        // pbr: setup framebuffer
        // ----------------------
        glGenFramebuffers(1, &captureFBO_);
        glGenRenderbuffers(1, &captureRBO_);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO_);

        glGenTextures(1, &envCubemap_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap_);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Game::envset<int>("env_cubemap", envCubemap_);

        ShaderData& envmap = Resources::getShaderData("envmap");
        // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
        // ----------------------------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        envmap.use();
        envmap.setMat4("projection", captureProjection);

        glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO_);
        for (unsigned int i = 0; i < 6; ++i)
        {
            envmap.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap_, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap_);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // then before rendering, configure the viewport to the original framebuffer's screen dimensions
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");
        glfwGetFramebufferSize(mainContext, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);
    }
    
    // renderCube() renders a 1x1 3D cube in NDC.
    // -------------------------------------------------
    unsigned int cubeVAO_ = 0;
    unsigned int cubeVBO_ = 0;
    void renderCube()
    {
        // initialize (if necessary)
        if (cubeVAO_ == 0)
        {
            float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
            };
            glGenVertexArrays(1, &cubeVAO_);
            glGenBuffers(1, &cubeVBO_);
            // fill buffer
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            // link vertex attributes
            glBindVertexArray(cubeVAO_);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        // render Cube
        glBindVertexArray(cubeVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // renderQuad() renders a 1x1 XY quad in NDC
    // -----------------------------------------
    unsigned int quadVAO_ = 0;
    unsigned int quadVBO_ = 0;
    void renderQuad()
    {
        if (quadVAO_ == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO_);
            glGenBuffers(1, &quadVBO_);
            glBindVertexArray(quadVAO_);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    // renders (and builds at first invocation) a sphere
    // -------------------------------------------------
    unsigned int sphereVAO_ = 0;
    unsigned int indexCount_;
    void renderSphere()
    {
        if (sphereVAO_ == 0)
        {
            glGenVertexArrays(1, &sphereVAO_);

            unsigned int vbo, ebo;
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);

            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> uv;
            std::vector<glm::vec3> normals;
            std::vector<unsigned int> indices;

            const unsigned int X_SEGMENTS = 64;
            const unsigned int Y_SEGMENTS = 64;
            const float PI = 3.14159265359f;
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
                {
                    float xSegment = (float)x / (float)X_SEGMENTS;
                    float ySegment = (float)y / (float)Y_SEGMENTS;
                    float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                    float yPos = std::cos(ySegment * PI);
                    float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                    positions.push_back(glm::vec3(xPos, yPos, zPos));
                    uv.push_back(glm::vec2(xSegment, ySegment));
                    normals.push_back(glm::vec3(xPos, yPos, zPos));
                }
            }

            bool oddRow = false;
            for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
            {
                if (!oddRow) // even rows: y == 0, y == 2; and so on
                {
                    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                    {
                        indices.push_back(y * (X_SEGMENTS + 1) + x);
                        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    }
                }
                else
                {
                    for (int x = X_SEGMENTS; x >= 0; --x)
                    {
                        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                        indices.push_back(y * (X_SEGMENTS + 1) + x);
                    }
                }
                oddRow = !oddRow;
            }
            indexCount_ = static_cast<unsigned int>(indices.size());

            std::vector<float> data;
            for (unsigned int i = 0; i < positions.size(); ++i)
            {
                data.push_back(positions[i].x);
                data.push_back(positions[i].y);
                data.push_back(positions[i].z);
                if (normals.size() > 0)
                {
                    data.push_back(normals[i].x);
                    data.push_back(normals[i].y);
                    data.push_back(normals[i].z);
                }
                if (uv.size() > 0)
                {
                    data.push_back(uv[i].x);
                    data.push_back(uv[i].y);
                }
            }
            glBindVertexArray(sphereVAO_);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
            unsigned int stride = (3 + 2 + 3) * sizeof(float);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        }

        glBindVertexArray(sphereVAO_);
        glDrawElements(GL_TRIANGLE_STRIP, indexCount_, GL_UNSIGNED_INT, 0);
    }


    void setupShadowMap() {
        glGenFramebuffers(1, &shadowMapFBO_);
        glGenTextures(1, &shadowMap_);
        glBindTexture(GL_TEXTURE_2D, shadowMap_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth_, shadowHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void updateShadowMap() {
        glViewport(0, 0, shadowWidth_, shadowHeight_);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO_);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void bindShadowMap() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap_);
    }

    void setupGBuffer() {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");
        // configure g-buffer framebuffer
        glGenFramebuffers(1, &forwardFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
        // position color buffer
        glGenTextures(1, &gPosition_);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition_, 0);
        // normal color buffer
        glGenTextures(1, &gNormal_);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal_, 0);
        // color + specular color buffer
        glGenTextures(1, &gAlbedoSpec_);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec_, 0);
        // world position color buffer
        glGenTextures(1, &gPositionWorld_);
        glBindTexture(GL_TEXTURE_2D, gPositionWorld_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPositionWorld_, 0);
        // world normal color buffer
        glGenTextures(1, &gNormalWorld_);
        glBindTexture(GL_TEXTURE_2D, gNormalWorld_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gNormalWorld_, 0);
        // depth texture
        glGenTextures(1, &gDepth_);
        glBindTexture(GL_TEXTURE_2D, gDepth_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gDepth_, 0);

        
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(6, attachments);
        
        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &forwardRBO_);
        glBindRenderbuffer(GL_RENDERBUFFER, forwardRBO_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, forwardRBO_);
        
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("Framebuffer not complete!", 1);
            
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void updateGBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void copyDepthBuffer() {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        glBindFramebuffer(GL_READ_FRAMEBUFFER, forwardFBO_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void bindGBuffer() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);
    }
    

    float ourLerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }
    
    void setupSsao() {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");
        // also create framebuffer to hold SSAO processing stage 
        // -----------------------------------------------------
        glGenFramebuffers(1, &ssaoFBO_);  glGenFramebuffers(1, &ssaoBlurFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
        // SSAO color buffer
        glGenTextures(1, &ssaoColorBuffer_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("SSAO Framebuffer not complete!", 1);
        // and blur stage
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO_);
        glGenTextures(1, &ssaoColorBufferBlur_);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur_, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Log::console("SSAO Blur Framebuffer not complete!", 1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // generate sample kernel
        // ----------------------
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;
        for (unsigned int i = 0; i < 64; ++i)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0f;

            // scale samples s.t. they're more aligned to center of kernel
            scale = ourLerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel_.push_back(sample);
        }

        // generate noise texture
        // ----------------------
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }

        ShaderData& shader = Resources::getShaderData("ssao");
        shader.use();
        for (unsigned int i = 0; i < 64; ++i)
                shader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);
        
        glGenTextures(1, &noiseTexture_);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    void updateSsao() {
        
        ShaderData& shaderSSAO = Resources::getShaderData("ssao");
        ShaderData& shaderSSAOBlur = Resources::getShaderData("ssao_blur");
        // 2. generate SSAO texture
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAO.use();
            // Send kernel + rotation 
            for (unsigned int i = 0; i < 64; ++i)
                shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);
            Drawer::prep("ssao");
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition_);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal_);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noiseTexture_);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 3. blur SSAO texture to remove noise
        // ------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO_);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAOBlur.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer_);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
    void bindSsaoGBuffer() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec_);
        glActiveTexture(GL_TEXTURE4); // add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur_);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gPositionWorld_);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, gNormalWorld_);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, gDepth_);
    }

    void setupSsaoCompute() {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        // Create SSAO compute shader output texture
        glGenTextures(1, &ssaoComputeOutput_);
        glBindTexture(GL_TEXTURE_2D, ssaoComputeOutput_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Create SSAO blur compute shader output texture
        glGenTextures(1, &ssaoComputeOutputBlur_);
        glBindTexture(GL_TEXTURE_2D, ssaoComputeOutputBlur_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Generate sample kernel
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
        std::default_random_engine generator;
        for (unsigned int i = 0; i < 64; ++i) {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0f;
            scale = ourLerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel_.push_back(sample);
        }

        // Generate noise texture
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 16; i++) {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
            ssaoNoise.push_back(noise);
        }

        glGenTextures(1, &noiseTexture_);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void updateSsaoCompute() {
        ComputeData& computeSSAO = Resources::getComputeData("ssao");

        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        computeSSAO.use();
        computeSSAO.setInt("kernelSize", 64);
        computeSSAO.setFloat("radius", 0.5f);
        computeSSAO.setFloat("bias", 0.025f);
        computeSSAO.setVec2("noiseScale", glm::vec2(screenWidth / 4.0, screenHeight / 4.0));
        glm::mat4 projection = Game::envget<glm::mat4>("projection");
        computeSSAO.setMat4("projection", projection);

        for (unsigned int i = 0; i < 64; ++i) {
            computeSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);
        }

        glBindImageTexture(0, ssaoComputeOutput_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture_);

        // Dispatch SSAO compute shader
        computeSSAO.dispatch((screenWidth + 15) / 16, (screenHeight + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Dispatch SSAO blur compute shader
        ComputeData& computeSSAOBlur = Resources::getComputeData("ssao_blur");
        computeSSAOBlur.use();
        glBindImageTexture(0, ssaoComputeOutput_, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, ssaoComputeOutputBlur_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        computeSSAOBlur.dispatch((screenWidth + 15) / 16, (screenHeight + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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
    void updateTime() const {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        Game::envset<float>("time.current", currentTime);
        Game::envset<float>("time.delta", deltaTime);

        Game::envset<float>("mouse.offsetX", MathUtils::lerp(Game::envget<float>("mouse.offsetX"), 0.0f, deltaTime * 5.0f));
        Game::envset<float>("mouse.offsetY", MathUtils::lerp(Game::envget<float>("mouse.offsetY"), 0.0f, deltaTime * 5.0f));
    }
    void clear(bool isClearColor=false) {
        if (isClearColor)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

        //Log::console("axesX: " + std::to_string(axesX) + ", axesY: " + std::to_string(axesY));
    }
    

    unsigned int getShadowBuffer() {
        return shadowMapFBO_;
    }
    unsigned int getShadowTexture() {
        return shadowMap_;
    }

    unsigned int getForwardBuffer() {
        return forwardFBO_;
    }
    unsigned int getPositionTexture() {
        return gPosition_;
    }
    unsigned int getNormalTexture() {
        return gNormal_;
    }
    unsigned int getAlbedoSpecTexture() {
        return gAlbedoSpec_;
    }
    unsigned int getDepthTexture() {
        return gDepth_;
    }
    unsigned int getPositionWorldTexture() {
        return gPositionWorld_;
    }
    unsigned int getNormalWorldTexture() {
        return gNormalWorld_;
    }
    unsigned int getNoiseTexture() {
        return noiseTexture_;
    }

    unsigned int getSsaoBuffer() {
        return ssaoFBO_;
    }
    unsigned int getSsaoTexture() {
        return ssaoColorBuffer_;
    }

    unsigned int getSsaoBlurBuffer() {
        return ssaoBlurFBO_;
    }
    unsigned int getSsaoBlurTexture() {
        return ssaoColorBufferBlur_;
    }

    unsigned int getSsaoComputeBlurTexture() {
        return ssaoComputeOutputBlur_;
    }

    std::vector<glm::vec3>& getSsaoKernel() {
        return ssaoKernel_;
    }

private:
    static inline float lastX_ = 0.0f;
    static inline float lastY_ = 0.0f;
    static inline bool firstMouse_ = true;

    unsigned int shadowMapFBO_, shadowMap_;
    unsigned int shadowWidth_ = 1024, shadowHeight_ = 1024;

    unsigned int ssaoFBO_, ssaoBlurFBO_;
    unsigned int noiseTexture_; 
    unsigned int ssaoColorBuffer_, ssaoColorBufferBlur_;

    unsigned int captureFBO_, captureRBO_, envCubemap_;

    unsigned int forwardFBO_, gDepth_, gPosition_, gPositionWorld_, gNormalWorld_, gNormal_, gAlbedoSpec_, forwardRBO_;
    std::vector<glm::vec3> ssaoKernel_;

    unsigned int ssaoComputeOutput_, ssaoComputeOutputBlur_;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Game::envset("screen.width", width);
        Game::envset("screen.height", height);
    }

    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
        // Implement mouse position handling logic here
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

        //Log::console("mouse_x: " + std::to_string(xoffset) + ", mouse_y: " + std::to_string(yoffset));
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        // Implement scroll handling logic here

        Game::envset("scroll_y", static_cast<float>(yoffset));
    }
};

#endif // WINDOW_H