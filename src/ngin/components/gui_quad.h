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
        : IDrawer(name, lex, parent), whiteTexture_(0), texture_(0) {
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
            std::cout << "GUI QUAD White Texture ID: " << whiteTexture_ << std::endl;   
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
    void draw(ShaderData& shader) override {
        int screenWidth = Ngin::envget<int>("screen.width"); // Example screen width
        int screenHeight = Ngin::envget<int>("screen.height"); // Example screen height

        if (texture_ != 0) {
            bindTexture();
        } else {
            bindWhiteTexture();
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
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture_);
    }

    void generateWhiteTexture() {
        glGenTextures(1, &whiteTexture_);
        glBindTexture(GL_TEXTURE_2D, whiteTexture_);

        unsigned char whitePixel[4] = { 255, 255, 255, 255 }; // RGBA white
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    void bindWhiteTexture() {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, whiteTexture_);
    }

    unsigned int quadVAO_ = 0;
    unsigned int quadVBO_ = 0;
    void renderQuad()
    {
        if (quadVAO_ == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, // Top-left corner
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Bottom-left corner
                 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, // Top-right corner
                 1.0f, -1.0f, 0.0f, 1.0f, 1.0f  // Bottom-right corner
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO_);
            glGenBuffers(1, &quadVBO_);
            glBindVertexArray(quadVAO_);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

            // Bind position attribute (location = 0)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

            // Bind texture coordinates attribute (location = 2)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    
};

#endif // GUI_QUAD_H
