#ifndef GL_CUBEMAP_H
#define GL_CUBEMAP_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/node/node.h>
#include <ngin/data/render_data.h>
#include <ngin/data/mesh_data.h>
#include <ngin/game.h>
#include <ngin/resources.h>

class GlCubemap : public Node {
public:
    explicit GlCubemap(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), cube_(Resources::getMeshData("cube")) {
    }

    ~GlCubemap() override = default;

    void update(std::string& pass) override {
        Node::update(pass);

        std::shared_ptr<NodePort> inputPortRenderGui = getInputPortByType(pass);
        if (!inputPortRenderGui) {
            Log::console("No input port found for pass: " + pass);
            return;
        }
        std::shared_ptr<RenderData> renderData = inputPortRenderGui->getData<RenderData>();
        if (renderData) {
            // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
            // ----------------------------------------------------------------------------------------------
            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] =
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
            };
            
            ShaderData& shaderData = renderData->getShader();
            shaderData.use();
            shaderData.setMat4("projection", captureProjection);

            unsigned int captureFBO = Game::env<int>("captureFBO");

            glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            for (unsigned int i = 0; i < 6; ++i)
            {
                shaderData.setMat4("view", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap_, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                cube_.render();
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        } else {
            Log::console("No shader data found for pass: " + pass);
        }
    }

    void start(std::string& pass) override {
        Node::start(pass);

        glGenTextures(1, &cubemap_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

private:
    unsigned int cubemap_;

    MeshData& cube_;
};

#endif // GL_CUBEMAP_H
