#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>

#include <ngin/data/mesh_data.h>
#include <ngin/log.h>

#include <ngin/drawer.h>    
#include <ngin/resources.h>    

class Mesh : public IDrawer {
public:
    Mesh(const std::string name, const Lex& lex, IObject* parent)
        : IDrawer(name, lex, parent), meshData_(Resources::getMeshData(lex.getC<std::string>("mesh", ""))), texture_(0) {
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        metallic_ = lex.getC<float>("metallic", 0.0);
        roughness_ = lex.getC<float>("roughness", 1.0);
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }
    }
    ~Mesh() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
    }

    void init() override {
        // Implementation of init method
        // load shader

        if (lex_.contains("texture")) {
            texture_ = Resources::getTextureId(lex_.getC<std::string>("texture", ""));
        }
    }

    void launch() override {
        // Implementation of launch method
    }

    void update() override {
        // Implementation of update method
    }

    void updateLate() override {
        // Implementation of updateLate method
    }
    void prep(ShaderData& shader) override {
    }


    void draw(ShaderData& shader) override {
        if (texture_ != 0) {
            bindTexture();
        } else {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        TransformData* transform = getTransform();
        glm::mat4 model = transform->getWorldModelMatrix();
        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setFloat("metallic", metallic_);
        shader.setFloat("roughness", roughness_);
        meshData_.render();
    }

private:
    MeshData& meshData_;
    unsigned int texture_;

    float metallic_, roughness_;

    void bindTexture() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_);
    }
};

#endif // LIGHT_H
