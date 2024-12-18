#ifndef MODEL_DRAWER_H
#define MODEL_DRAWER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ngin/resources.h>
#include <ngin/log.h>
#include <ngin/nodes/i_drawer.h> // Include IDrawer
#include <ngin/data/mesh_data.h> // Include MeshData
#include <ngin/drawer.h> // Include IDrawer

#include <memory>
#include <vector>

class ModelDrawer : public IDrawer, public std::enable_shared_from_this<ModelDrawer> { // Inherit from IDrawer and enable_shared_from_this
public:
    explicit ModelDrawer(const std::string& name, Nevf& dictionary)
        : IDrawer(name, dictionary) // Initialize IDrawer
    {
        model_ = dictionary.getC<std::string>("model", "");
        passes_ = dictionary.getC<std::vector<std::string>>("passes", {});    
    }

    ~ModelDrawer() override {
        for (const auto& pass : passes_) {
            Drawer::unregisterDrawer(pass, ModelDrawer::shared_from_this());
        }
    }

    void setup() override { 
        IDrawer::setup();

        data_ = std::make_shared<MeshData>(model_);
    }

    void launch() override { 
        IDrawer::launch();

        for (const auto& pass : passes_) {
            Drawer::registerDrawer(pass, ModelDrawer::shared_from_this());
        }
    }

    void execute(std::string& pass) override {
        IDrawer::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    }

    void render(ShaderData& shader) override {
        // Implement the render method
        shader.setMat4("M_MODEL", getTransformation());
        data_->render();
    }

protected:
    std::string model_;
    std::vector<std::string> passes_;

    std::shared_ptr<MeshData> data_;

    glm::mat4 getTransformation() {
        return glm::mat4(1.0f);
    }
};

#endif  // MODEL_DRAWER_H
