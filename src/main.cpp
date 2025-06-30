#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ngin/util/file.h>

#include <iostream>
#include <vector>
#include <memory>

#include "Headers/Shader.h"
#include "Headers/Model.h"
#include "Headers/cyTriMesh.h"
#include "Headers/Camera.h"
#include "Headers/Quad.h"

#include <ngin/render/gl/data.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos);

#pragma region parameters
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0, 0, 30.0f));
float cameraSpeed = 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

const char * modelNames[] = {
    "bunny",
    "SIMPLE ROUND TABLE",
    "teapot",
    "cube"
};
glm::vec3 modelTransformation[] = {
    glm::vec3(0.0f, 5.0f, -20.0f), glm::vec3(3.0f, 3.0f, 3.0f), //translation, scale
    glm::vec3(-17.0f, -9.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(-17.0f, -2.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(0.0f, -9.0f, 0.0f), glm::vec3(0.6f, 1.0f, 0.6f),
};

bool flipModel[] = {
    false,
    false,
    true,
    false,
};
#pragma endregion
glm::vec3 lights[] = {
    glm::vec3(20, 10, 10),    glm::vec3(1,1,1),               glm::vec3(1,0.007f,0.0002f),          //position, color, (constant, linear, quadratic)
    glm::vec3(-25, -5, -35), glm::vec3(0.224f,0.42f,0.659f), glm::vec3(1,0.007f,0.0002f),
    glm::vec3(25, -5, -35), glm::vec3(0.306f,0.714f,0.71f), glm::vec3(1,0.027,0.0028),
};

class NginApplication {
public:
    void run() {
        init_();
        run_();
        cleanup_();
    }

private:
    GlData data_;

    Model* ground_;
    Quad* quad_;

    std::vector<Model*> models_;

    Shader* shader_forward_;
    Shader* shader_lp_;
    Shader* shader_ssr_;
    Shader* shader_output_;

    void init_() {
    #pragma region OpenGl Initialization
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);
        //glfwWindowHint(GLFW_STENCIL_BITS, 8);
        data_.window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SSR", NULL, NULL);
        if (data_.window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(data_.window);
        glfwSetFramebufferSizeCallback(data_.window, framebuffer_size_callback);
        glfwSetCursorPosCallback(data_.window, mouseMovementCallback);
        glfwSetInputMode(data_.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);    

        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &data_.original_fb);
    #pragma endregion

        data_.projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);

        quad_ = new Quad();

    #pragma region GBuffer Initialization
        glGenFramebuffers(1, &data_.forward_fb);
        glBindFramebuffer(GL_FRAMEBUFFER, data_.forward_fb);

        glGenTextures(1, &data_.tex_g_normal);
        glBindTexture(GL_TEXTURE_2D, data_.tex_g_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_.tex_g_normal, 0);
        glGenTextures(1, &data_.tex_g_albedo);
        glBindTexture(GL_TEXTURE_2D, data_.tex_g_albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, data_.tex_g_albedo, 0);
        glGenTextures(1, &data_.tex_g_specular);
        glBindTexture(GL_TEXTURE_2D, data_.tex_g_specular);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, data_.tex_g_specular, 0);

        //depth texture
        glGenTextures(1, &data_.tex_depthmap);
        glBindTexture(GL_TEXTURE_2D, data_.tex_depthmap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, data_.tex_depthmap, 0);
    
        GLenum drawBuffers1[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, drawBuffers1);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            return;
        }
    #pragma endregion

    #pragma region lighting pass Buffer Initialization
        glGenFramebuffers(1, &data_.deferred_fb);
        glBindFramebuffer(GL_FRAMEBUFFER, data_.deferred_fb);

        glGenTextures(1, &data_.tex_color);
        glBindTexture(GL_TEXTURE_2D, data_.tex_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_.tex_color, 0);
        
        GLenum drawBuffers2[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers2);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "problems binding data_.deferred_fb" << std::endl;
            return;
        }
    #pragma endregion

    #pragma region SSR pass Buffer Initialization
        glGenFramebuffers(1, &data_.ssr_fb);
        glBindFramebuffer(GL_FRAMEBUFFER, data_.ssr_fb);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, data_.tex_depthmap, 0);

        glGenTextures(1, &data_.tex_color_reflection);
        glBindTexture(GL_TEXTURE_2D, data_.tex_color_reflection);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_.tex_color_reflection, 0);
        
        GLenum drawBuffers3[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers3);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "problems binding data_.ssr_fb" << std::endl;
            return;
        }
    #pragma endregion

        shader_forward_ = new Shader((FileUtil::get_resource_path("Shaders/ForwardPassVS.vs")).c_str(), (FileUtil::get_resource_path("Shaders/ForwardPassFS.fs")).c_str());

        int numOfModels = 3;
        cyTriMesh* Objectctm = new cyTriMesh;
        for (int i = 0; i < numOfModels; i++)
        {
            if (!Objectctm->LoadFromFileObj((FileUtil::get_resource_path("Models/" + std::string(modelNames[i]) + ".obj").c_str()))) {
                return;
            }
            models_.push_back(new Model(*Objectctm, *shader_forward_, modelTransformation[i*2], modelTransformation[i*2+1], data_.projection, true, flipModel[i]));
        }
        delete Objectctm;

        cyTriMesh Groundctm;
        if (!Groundctm.LoadFromFileObj(FileUtil::get_resource_path("Models/ground.obj").c_str())) {
            return;
        }   
        ground_ = new Model(Groundctm, *shader_forward_, glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f), data_.projection, true, false); //ground

        shader_lp_ = new Shader((FileUtil::get_resource_path("Shaders/DeferredPassVS.vs")).c_str(), (FileUtil::get_resource_path("Shaders/DeferredPassFS.fs")).c_str());
        shader_lp_->use();
        shader_lp_->setInt("gNormal", 0);
        shader_lp_->setInt("gAlbedo", 1);
        shader_lp_->setInt("gSpecular", 2);
        shader_lp_->setInt("depthMap", 3);
        for (int i = 0; i < sizeof(lights) / sizeof(lights[0]); i++)
        {
            shader_lp_->setVec3("lights[" + std::to_string(i) + "].intensity", lights[i * 3 + 1]);
            shader_lp_->setFloat("lights[" + std::to_string(i) + "].constant", lights[i * 3 + 2].x);
            shader_lp_->setFloat("lights[" + std::to_string(i) + "].linear", lights[i * 3 + 2].y);
            shader_lp_->setFloat("lights[" + std::to_string(i) + "].quadratic", lights[i * 3 + 2].z);
        }

        shader_ssr_ = new Shader((FileUtil::get_resource_path("Shaders/SSRVS.vs")).c_str(), ((FileUtil::get_resource_path("Shaders/SSRFS.fs"))).c_str());
        shader_ssr_->use();
        shader_ssr_->setInt("gNormal", 0);
        shader_ssr_->setInt("colorBuffer", 1);
        shader_ssr_->setInt("depthMap", 2);

        shader_output_ = new Shader((FileUtil::get_resource_path("Shaders/SSRVS.vs")).c_str(), (FileUtil::get_resource_path("Shaders/outputFS.fs")).c_str());
        shader_output_->use();
        shader_output_->setInt("colorTexture", 0);
        shader_output_->setInt("refTexture", 1);
        shader_output_->setInt("specularTexture", 2);

        data_.view = glm::mat4(1.0);
    }

    void run_() {
        while (!glfwWindowShouldClose(data_.window)) {
            processInput(data_.window);
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;


            //Forward Pass
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data_.forward_fb);
            glClearColor(0, 0, 0, 1.0f);
            glStencilMask(0xFF);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glStencilFunc(GL_EQUAL, 0, 0xFF);
            glStencilMask(0x00); 
            data_.view = camera.GetViewMatrix();
            for (Model* m : models_)
            {   
                m->Draw(data_.view);
            }

            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
            ground_->Draw(data_.view);
            //sphere->setLightPosition(view* glm::vec4(lightPosition, 1));
            //sphere->Draw(view);

            //Deferred(Lighting) Pass
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data_.deferred_fb);
            glClearColor(0, 0, 0, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, data_.tex_g_normal);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, data_.tex_g_albedo);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, data_.tex_g_specular);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, data_.tex_depthmap);

            shader_lp_->use();
            shader_lp_->setVec3("lightPosition", data_.view * glm::vec4(lights[0], 1));
            shader_lp_->setFloat("SCR_WIDTH", SCR_WIDTH);
            shader_lp_->setFloat("SCR_HEIGHT", SCR_HEIGHT);
            shader_lp_->setMat4("invProj", glm::inverse(data_.projection));
            for (int i = 0; i < sizeof(lights) / sizeof(lights[0]); i++)
            {     
                shader_lp_->setVec3("lights[" + std::to_string(i) + "].position", data_.view * glm::vec4(lights[i * 3], 1));
            }

    
            glBindVertexArray(quad_->GetVAO());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            //SSR
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data_.ssr_fb);
            glClearColor(0, 0, 0, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDepthMask(GL_FALSE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, data_.tex_g_normal);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, data_.tex_color);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, data_.tex_depthmap);

            shader_ssr_->use();
            shader_ssr_->setFloat("SCR_WIDTH", SCR_WIDTH);
            shader_ssr_->setFloat("SCR_HEIGHT", SCR_HEIGHT);
            shader_ssr_->setMat4("invProjection", glm::inverse(data_.projection));
            shader_ssr_->setMat4("projection", data_.projection);

            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glBindVertexArray(quad_->GetVAO());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glDepthMask(GL_TRUE);

            //output
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data_.original_fb);
            glClearColor(0, 0, 0, 1.0f);
            glStencilMask(0xFF);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, data_.tex_color);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, data_.tex_color_reflection);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, data_.tex_g_specular);

            shader_output_->use();
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glBindVertexArray(quad_->GetVAO());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(data_.window);

            glfwPollEvents();
        }
    }

    void cleanup_() {
        for (Model* m : models_)
        {
            delete m;
        }
        delete ground_;
        delete quad_;

        delete shader_forward_;
        delete shader_lp_;
        delete shader_ssr_;
        delete shader_output_;

        glDeleteFramebuffers(1, &data_.forward_fb);
        glDeleteFramebuffers(1, &data_.deferred_fb);
        glDeleteFramebuffers(1, &data_.ssr_fb);

        glfwTerminate();
    }
};

int main() {
    NginApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move(FORWARD, deltaTime * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move(BACKWARD, deltaTime * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(LEFT, deltaTime * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(RIGHT, deltaTime * cameraSpeed);
    }
}

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    camera.rotate(xpos, ypos);
}
