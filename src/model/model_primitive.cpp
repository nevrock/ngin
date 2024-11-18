#include "model_primitive.h"
#include <iostream>
#include "../light/light.h"
#include <ngin/constants.h>

ModelPrimitive::ModelPrimitive(Object* parent) : Model(parent) {}

ModelPrimitive::~ModelPrimitive() {
}

void ModelPrimitive::loadFromNevf(const Nevf& d) {
    fileName = d.getC<std::string>("type", "");
    typeName = fileName;
    data = Resources::getModel(fileName);

    isInstance = d.getC<bool>("is_instance", false);
    isCull = d.getC<bool>("is_cull", false);
    layer = d.getC<int>("layer", 0);

    distanceLimit = d.getC<float>("distance", 25.0);

    int texIndex = 2;
    if (d.contains("textures")) {
        for (const auto& tex : d.getC<std::vector<std::string>>("textures", std::vector<std::string>{""})) {
            if (tex == "") continue;

            Texture texture;
            texture.id = texIndex,
            texture.name = tex;

            textures.push_back(texture);

            texIndex += 1;
        }
    }
}
void ModelPrimitive::updatePreRender(const unsigned int index, Shader& shader) {
}
void ModelPrimitive::updateRender(const unsigned int index, Shader& shader) {
    if (index >= ngin::RENDER_LAYER_DIRECTIONAL_DEPTH && index <= 2) {
        shader.setInt("NUM_LIGHTS", 0);

        if ((index == ngin::RENDER_LAYER_POINT_DEPTH || index == 2)) {
            shader.setInt("NUM_LIGHTS", 1);

            glm::vec3 pos = getPoint().getPosition();
            Light* l = Light::getClosestPointLight(pos);
            if (l != nullptr) {
                l->updateShadows(shader);
                shader.setInt("NUM_LIGHTS", 1);
            } 
        }
        shader.setMat4("M_MODEL", getPoint().getModelMatrix());
        shader.setFloat("POINT_SHADOWS_THRESHOLD", distanceLimit);
        shader.setFloat("IS_ANIMATION", false);
        if (index == 2) {
            int texIndex = 2;  // This represents the base texture unit (GL_TEXTURE2)
            for (int i = 0; i < textures.size(); i++) {
                // Retrieve the texture by its name
                Texture2D tex = Resources::getTexture(textures[i].name);
                GLenum textureUnit = GL_TEXTURE0 + (texIndex + i);  // Calculate texture unit dynamically
                glActiveTexture(textureUnit);
                tex.bind();
                // Dynamically set the shader uniform to point to the right texture unit
                shader.setInt("TEX_DIFFUSE", texIndex + i);
            }
        }
        data->render();
    }
}

bool ModelPrimitive::isValidLayer(const unsigned int index) {
    if (index >= 0 && index <= 2) {
        return true;
    } return false;
}


bool model_primitive_registered = []() {
    Object::registerComponent("model_primitive", [](Object* parent) {
        return std::make_shared<ModelPrimitive>(parent);
    });
    return true;
}();