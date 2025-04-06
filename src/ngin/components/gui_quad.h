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
        : IDrawer(name, lex, parent) {
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }
    }
    ~GuiQuad() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
    }

    void init() override {
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
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
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
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    void draw(ShaderData& shader) override {
        int screenWidth = Ngin::envget<int>("screen.width"); // Example screen width
        int screenHeight = Ngin::envget<int>("screen.height"); // Example screen height

        glm::mat4 model = getRectTransform()->getModelMatrix();
        shader.setMat4("model", model);
        shader.setBool("isText", false);

        renderQuad();
    }

private:
};

#endif // GUI_QUAD_H
