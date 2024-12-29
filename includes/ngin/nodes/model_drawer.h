#ifndef MODEL_DRAWER_H
#define MODEL_DRAWER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ngin/resources.h>
#include <ngin/nodes/i_drawer.h> // Include IDrawer
#include <ngin/data/mesh_data.h> // Include MeshData
#include <ngin/drawer.h> // Include IDrawer

#include <memory>
#include <vector>

class ModelDrawer : public IDrawer { // Inherit from IDrawer and enable_shared_from_this
public:
    explicit ModelDrawer(const std::string& name, Nevf& dictionary)
        : IDrawer(name, dictionary), model_(dictionary.getC<std::string>("model", "")),
        meshData_(Resources::getMeshData(model_)), passes_(dictionary.getC<std::vector<std::string>>("passes", {})) {

    }

    ~ModelDrawer() override {
        for (const auto& pass : passes_) {
            Log::console("unregistering drawer: " + pass);
            Drawer::unregisterDrawer(pass, this);
        }
    }

    void setup() override { 
        IDrawer::setup();
    }

    void start(std::string& pass) override { 
        IDrawer::start(pass);

        for (const auto& pass : passes_) {
            Log::console("registering drawer: " + pass);
            Drawer::registerDrawer(pass, this);
        }
    }

    void update(std::string& pass) override {
        IDrawer::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
        transforms_.clear();
        // this will get called on a transform pass
        std::vector<std::shared_ptr<NodePort>> ports = getInputPortsByType("transform");
        for (const auto& port : ports) {
            std::shared_ptr<TransformData> data = port->getData<TransformData>();
            if (data) {
                transforms_.push_back(data);
            }
        }
    }

    void render(ShaderData& shader) override {
        for (const auto& transform : transforms_) {
            shader.setMat4("M_MODEL", transform->getWorldModelMatrix());
            meshData_.render();
        }
    }

protected:
    std::string model_;
    std::vector<std::string> passes_;

    MeshData& meshData_;
    std::vector<std::shared_ptr<TransformData>> transforms_;

};

#endif  // MODEL_DRAWER_H
