#ifndef MODEL_H
#define MODEL_H

#include <snorri/object.h>
#include <snorri/mesh.h>
#include <snorri/resources.h>
#include <snorri/model_drawer.h>
#include <snorri/log.h>
#include <memory>
#include <vector>

struct ModelData {
public:
    std::string name;
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<Texture> textures;

    Mesh* getMesh(int index) {
        if (index < meshes.size()) {
            return meshes[index].get();
        } return nullptr;
    }

    void render() {
        for (auto& mesh : meshes) {
            mesh->render();
        }
    }
    void setName(std::string name) { 
        this->name = name;
    }
};

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
};

#endif  // MODEL_H
