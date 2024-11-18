#ifndef MODEL_SDF_H
#define MODEL_SDF_H

#include <ngin/model.h>
#include <ngin/surface_data.h>

class ModelSdf : public Model {
public:
    ModelSdf(Object* parent);
    ~ModelSdf() override;
    void loadFromDict(const Dict& d) override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateRender(const unsigned int index, Shader& shader) override;
    bool isValidLayer(const unsigned int index) override;
private:
    void initializeBuffers();
    unsigned int updateBuffers();

    GLuint surfacesBuffer;
    unsigned int currentBufferSize;
};

#endif  // MODEL_H
