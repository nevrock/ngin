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

class ModelDrawer : public IDrawer { // Inherit from IDrawer and enable_shared_from_this
public:
    explicit ModelDrawer(const std::string& name, Nevf& dictionary)
        : IDrawer(name, dictionary) // Initialize IDrawer
    {
        model_ = dictionary.getC<std::string>("model", "");
        passes_ = dictionary.getC<std::vector<std::string>>("passes", {});    
    }

    ~ModelDrawer() override {
        for (const auto& pass : passes_) {
            Log::console("Unregistering drawer: " + pass);
            Drawer::unregisterDrawer(pass, this);
        }
    }

    void setup() override { 
        IDrawer::setup();

        data_ = Resources::loadMeshData(model_);
        data_->log();
    }

    void launch() override { 
        IDrawer::launch();

        for (const auto& pass : passes_) {
            Log::console("Registering drawer: " + pass);
            Drawer::registerDrawer(pass, this);
        }
    }

    void update(std::string& pass) override {
        IDrawer::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    }

    void render(std::shared_ptr<ShaderData> shader) override {
        if (!shader) {
            Log::console("Shader is null in model drawer render");
            return;
        }
        Log::console("Rendering model: " + model_ + " with shader: " + shader->getName());
        //return;
        // Implement the render method
        shader->setMat4("M_MODEL", getTransformation());
        if (data_) {
            data_->render();
        }
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
