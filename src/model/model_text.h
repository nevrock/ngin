#ifndef MODEL_TEXT_H
#define MODEL_TEXT_H

#include <ngin/gl/model.h>
#include <ngin/constants.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // If not already included elsewhere for matrix operations
#include <string>
#include <map>

// Forward declaration to avoid including whole font header
class Font;

class ModelText : public Model {
public:
    ModelText(Object* parent);
    ~ModelText() override;

    unsigned int VAO, VBO;  // Vertex Array Object and Vertex Buffer Object for text rendering

    void loadFromNevf(const Nevf& d) override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateRender(const unsigned int index, Shader& shader) override;
    void draw(Shader &shader, std::string text, float x, float y, float scale) const;
    glm::mat4 getTransformation() override;

    int getSortIndex() override;

    bool isValidLayer(const unsigned int index) override;

private:
    std::string text;  // The text content to be displayed
    std::string fontName;  // Name of the font used for the text
};

#endif  // MODEL_TEXT_H
