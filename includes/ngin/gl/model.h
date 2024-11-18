#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <vector>

#include <ngin/scene/object.h>
#include <ngin/gl/mesh.h>
#include <ngin/resources.h>
#include <ngin/gl/model_drawer.h>
#include <ngin/log.h>

class Model : public Component {
public:
    Model(IObject* parentObj) : Component(parentObj) {
    }
    ~Model() override { // Destructor for Model
        // Automatic cleanup code for Model specific resources
        if (isAddedToDrawer)
            ModelDrawer::removeModel(fileName, layer, this);
    }

    void launch() override {
        if (isCull)
            return;

        ModelDrawer::addModel(fileName, layer, this);
        isAddedToDrawer = true;
        Log::console(":::model added to drawer! " + fileName);
    }
    
    virtual glm::mat4 getTransformation() {
        IObject* p = getParent();
        if (p == nullptr) {
            return getPoint().getModelMatrix();
        }
        return p->getWorldMatrix();
    }
    virtual glm::vec2 getUvOffset() {
        return glm::vec2(0,0);
    }
    virtual glm::vec3 getColor() {
        return glm::vec3(1,1,1);
    }
    virtual bool isValidLayer(const unsigned int index) {
        return false;
    }
    virtual int getSortIndex() {
        return getParent()->getHierarchyIndex();
    }

    bool isInstance;
    bool isCull;

protected:
    ModelData* data;
    std::vector<Texture> textures;
    std::string fileName;
    unsigned int layer = 0;
    bool isAddedToDrawer;
    bool isTextured;
};

#endif  // MODEL_H
