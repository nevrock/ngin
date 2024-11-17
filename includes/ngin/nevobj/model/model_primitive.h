#ifndef MODEL_PRIMITIVE_H
#define MODEL_PRIMITIVE_H

#include <snorri/model.h>

class ModelPrimitive : public Model {
public:
    ModelPrimitive(Object* parent);
    ~ModelPrimitive() override;
    void loadFromDict(const Dict& d) override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateRender(const unsigned int index, Shader& shader) override;
    bool isValidLayer(const unsigned int index) override;
private:
    std::string typeName;
    float distanceLimit;
    std::vector<Texture> textures;
};

#endif  // MODEL_H
