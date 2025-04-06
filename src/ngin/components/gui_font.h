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

        verticalAlignmentTop_ = lex.getC<bool>("verticalAlignmentTop", false);
        horizontalAlignmentLeft_ = lex.getC<bool>("horizontalAlignmentLeft", true);
        color_ = lex.getVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        scale_ = lex.getC<float>("scale", 1.0f);
        font_ = lex.getC<std::string>("font", "header");
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

    void renderText(std::string text) const {
        FontData& font = Resources::getFont(font_);

        // Calculate the total width and height of the text
        auto [textWidth, textHeight] = getTextBounds(text, font);
        textWidth *= scale_;
        textHeight *= scale_;
        glm::mat4 rect = getRectTransform()->getRectMatrix();

        glm::vec2 rectOrigin = glm::vec2(rect[0][0], rect[0][1]);
        glm::vec2 rectSize = glm::vec2(rect[1][0], rect[1][1]);
        
        float x = rectOrigin.x;
        float y = rectOrigin.y;

        if (!horizontalAlignmentLeft_) {
            x += rectSize.x - textWidth;
        }

        if (verticalAlignmentTop_) {
            y += rectSize.y - textHeight;
        }

        glBindVertexArray(vao_);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, font.id); // Bind the entire texture atlas

        for (const char& c : text) {
            Character ch = font.getCharacter(c);

            float xpos = x + ch.Bearing.x * scale_;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale_;
            float zpos = 0.0f; // Z component is 0 for all vertices

            float w = ch.Size.x * scale_;
            float h = ch.Size.y * scale_;

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
            x += (ch.Advance >> 6) * scale_; // Advance the cursor
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void draw(ShaderData& shader) override {
        int screenWidth = Ngin::envget<int>("screen.width"); // Example screen width
        int screenHeight = Ngin::envget<int>("screen.height"); // Example screen height

        //glm::mat4 model = getRectTransform()->getWorldModelMatrix();
        //glm::vec2 anchor = getRectTransform()->getAnchor();

        //shader.setMat4("model", model);
        
        shader.setBool("isText", true);
        shader.setVec4("COLOR", color_); // Example color    
        
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
        shader.setMat4("projection", projection);
        
        renderText("Hello, World!"); // Example text rendering
    }

private:
    bool verticalAlignmentTop_;
    bool horizontalAlignmentLeft_;
    glm::vec4 color_;
    float scale_;
    std::string font_;

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

    std::pair<float, float> getTextBounds(const std::string& text, const FontData& font) const {
        float textWidth = 0.0f;
        float textHeight = 0.0f;
        for (const char& c : text) {
            Character ch = font.getCharacter(c);
            textWidth += (ch.Advance >> 6); // Advance is in 1/64th pixels
            textHeight = std::max(textHeight, static_cast<float>(ch.Size.y));
        }
        return {textWidth, textHeight};
    }
};

#endif // GUI_FONT_H
