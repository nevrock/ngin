#ifndef GUI_QUAD_H
#define GUI_QUAD_H

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ngin/data/mesh_data.h>
#include <ngin/log.h>
#include <ngin/ngin.h>

#include <ngin/drawer.h>    
#include <ngin/resources.h>    

class GuiQuad : public IDrawer {
public:
    GuiQuad(const std::string name, const Lex& lex, IObject* parent)
        : IDrawer(name, lex, parent), whiteTexture_(0) {
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }

        color_ = lex.getVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    ~GuiQuad() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
        if (whiteTexture_ != 0) {
            glDeleteTextures(1, &whiteTexture_);
        }
    }

    void init() override {
        if (lex_.contains("texture")) {
            texture_ = Resources::getTextureId(lex_.getC<std::string>("texture", ""));
            std::cout << "GUI QUAD Texture ID: " << texture_ << std::endl;
        } else {
            generateWhiteTexture();
        }
    }
    void launch() override {
        // Implementation of launch method
    }
    void update() override {
        // Implementation of update method
    }
    void updateLate() override {
        // Implementation of updateLate method
    }
    void prep(ShaderData& shader) override {
    }

    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void renderQuad()
    {
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // top-right

                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f  // top-left
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    void draw(ShaderData& shader) override {
        int screenWidth = Ngin::envget<int>("screen.width"); // Example screen width
        int screenHeight = Ngin::envget<int>("screen.height"); // Example screen height

        if (texture_ != 0) {
            bindTexture();
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, whiteTexture_);
        }
        
        glm::mat4 model = getRectTransform()->getModelMatrix();
        shader.setMat4("model", model);
        shader.setBool("isText", false);
        shader.setVec4("COLOR", color_);

        renderQuad();
    }

private:
    glm::vec4 color_;
    unsigned int texture_;
    unsigned int whiteTexture_;

    void bindTexture() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);
    }

    void generateWhiteTexture() {
        glGenTextures(1, &whiteTexture_);
        glBindTexture(GL_TEXTURE_2D, whiteTexture_);

        unsigned char whitePixel[4] = { 255, 255, 255, 255 }; // RGBA white
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif // GUI_QUAD_H
