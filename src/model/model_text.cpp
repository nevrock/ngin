#include "model_text.h"
#include <ngin/constants.h>
#include <ngin/font.h>
#include <iostream>

ModelText::ModelText(Object* parent) : Model(parent) {}

ModelText::~ModelText() {
}

void ModelText::loadFromNevf(const Nevf& d) {
    fileName = std::string("text");
    isInstance = false;

    isCull = d.getC<bool>("is_cull", false);
    layer = d.getC<int>("layer", 0);

    fontName = d.getC<std::string>("font", "antonio_bold");
    text = d.getC<std::string>("text", "this is a test!");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Assuming each vertex has 3 floats for position and 2 floats for UVs, for 6 vertices
    glBufferData(GL_ARRAY_BUFFER, 6 * (3 + 2) * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV attribute
    glEnableVertexAttribArray(2);  // Assuming layout location 2 for UVs
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

glm::mat4 ModelText::getTransformation() {
    glm::mat4 worldTransform = parent->getWorldMatrix(false);
    glm::vec4 position = worldTransform * glm::vec4(parent->getPoint().getPosition(), 1);
    glm::mat4 matrixOut = parent->getPointUi().getUiMatrix(glm::vec2(position.x, position.y));
    return matrixOut;
}

void ModelText::updatePreRender(const unsigned int index, Shader& shader) {}

int ModelText::getSortIndex() {
    return 1;
}

void ModelText::updateRender(const unsigned int index, Shader& shader) {
    if (index == ngin::RENDER_LAYER_UI) {
        shader.setInt("NUM_LIGHTS", 0);
        shader.setInt("text", 2);
        shader.setMat4("M_MODEL", getTransformation());
        shader.setFloat("IS_ANIMATION", false);
        shader.setFloat("IS_TEXT", true);
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(ngin::SCREEN_WIDTH), 0.0f, static_cast<float>(ngin::SCREEN_HEIGHT));
        shader.setMat4("M_CAMERA_PROJECTION", projection);
        shader.setVec3("textColor", glm::vec3(0.5));

        glm::vec3 position = getParent()->getWorldPosition();

        draw(shader, text, position.x, position.y, 1.0);

        shader.setFloat("IS_TEXT", false);
    }
}

void ModelText::draw(Shader &shader, std::string text, float x, float y, float scale) const {
    Font& font = Resources::getFont(fontName);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, font.id);  // Bind the entire texture atlas

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = font.getCharacter(*c);

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float zpos = 0.0f;  // Z component is 0 for all vertices

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

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexData), vertexData);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; // Advance the cursor
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool ModelText::isValidLayer(const unsigned int index) {
    return index == ngin::RENDER_LAYER_UI;
}

bool model_text_registered = []() {
    Object::registerComponent("model_text", [](Object* parent) {
        return std::make_shared<ModelText>(parent);
    });
    return true;
}();
