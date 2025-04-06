#ifndef GUI_FONT_H
#define GUI_FONT_H

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ngin/data/font_data.h>
#include <ngin/log.h>
#include <ngin/ngin.h>

#include <ngin/drawer.h>    
#include <ngin/resources.h>    

class GuiFont : public IDrawer {
public:
    GuiFont(const std::string name, const Lex& lex, IObject* parent)
        : IDrawer(name, lex, parent) {
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }
    }
    ~GuiFont() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
    }

    void init() override {
        setupMesh();
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

    void renderText(std::string text, float x, float y, float scale, glm::vec2 anchor) const {
        FontData& font = Resources::getFont("pixel");

        // Calculate the total width and height of the text
        float textWidth = 0.0f;
        float textHeight = 0.0f;
        for (const char& c : text) {
            Character ch = font.getCharacter(c);
            textWidth += (ch.Advance >> 6) * scale; // Advance is in 1/64th pixels
            textHeight = std::max(textHeight, ch.Size.y * scale);
        }

        x -= textWidth * anchor.x;
        y -= textHeight * anchor.y;

        glBindVertexArray(vao_);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, font.id); // Bind the entire texture atlas

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = font.getCharacter(*c);

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
            float zpos = 0.0f; // Z component is 0 for all vertices

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            float vertexData[6 * 5] = {
                xpos,     ypos + h,   zpos, ch.uvTopLeft.x,     ch.uvTopLeft.y,
                xpos,     ypos,       zpos, ch.uvTopLeft.x,     ch.uvBottomRight.y,
                xpos + w, ypos,       zpos, ch.uvBottomRight.x, ch.uvBottomRight.y,
                xpos,     ypos + h,   zpos, ch.uvTopLeft.x,     ch.uvTopLeft.y,
                xpos + w, ypos,       zpos, ch.uvBottomRight.x, ch.uvBottomRight.y,
                xpos + w, ypos + h,   zpos, ch.uvBottomRight.x, ch.uvTopLeft.y
            };

            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexData), vertexData);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.Advance >> 6) * scale; // Advance the cursor
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void draw(ShaderData& shader) override {
        int screenWidth = Ngin::envget<int>("screen.width"); // Example screen width
        int screenHeight = Ngin::envget<int>("screen.height"); // Example screen height

        glm::mat4 model = getRectTransform()->getWorldModelMatrix();
        glm::vec2 anchor = getRectTransform()->getAnchor();

        shader.setMat4("model", model);
        
        shader.setBool("isText", true);
        
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
        shader.setMat4("projection", projection);
        
        renderText("Hello, World!", model[0][0], model[0][1], 1.0, glm::vec2(model[2][0], model[2][1])); // Example text rendering
    }

private:

    unsigned int vao_, vbo_;
    void setupMesh() {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
        glBufferData(GL_ARRAY_BUFFER, 6 * (3 + 2) * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
        glEnableVertexAttribArray(2); 
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

#endif // GUI_FONT_H
