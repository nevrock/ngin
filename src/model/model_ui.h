
#ifndef MODEL_UI_H
#define MODEL_UI_H

#include <ngin/gl/model.h>
#include <ngin/constants.h>

class ModelUi : public Model {
public:
    ModelUi(Object* parent);
    ~ModelUi() override;

    void loadFromNevf(const Nevf& d) override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateRender(const unsigned int index, Shader& shader) override;

    glm::mat4 getTransformation() override;
    bool isValidLayer(const unsigned int index) override;

private:
    bool isRenderChildren = false;
};

#endif  // MODEL_H
