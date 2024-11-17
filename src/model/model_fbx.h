#ifndef MODEL_FBX_H
#define MODEL_FBX_H

#include <snorri/model_animation.h>

class ModelFbx : public ModelAnimation {
public:
    ModelFbx(Object* parent);
    ~ModelFbx() override;

    void loadFromDict(const Dict& d) override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateRender(const unsigned int index, Shader& shader) override;

    bool isValidLayer(const unsigned int index) override;
private:
};

#endif  // MODEL_H
