#include "model_fbx.h"
#include <iostream>
#include "../light/light.h"
#include <snorri/resources.h>
#include <snorri/constants.h>

ModelFbx::ModelFbx(Object* parent) : ModelAnimation(parent) {}

ModelFbx::~ModelFbx() {
}

void ModelFbx::loadFromDict(const Dict& d) {
    fileName = d.getC<std::string>("file", "");
    data = Resources::getModel(fileName);

    isInstance = d.getC<bool>("is_instance", false);
    isCull = d.getC<bool>("is_cull", false);
    layer = d.getC<int>("layer", 0);

    int texIndex = 2;
    if (d.contains("textures")) {
        for (const auto& tex : d.getC<std::vector<std::string>>("textures", std::vector<std::string>{""})) {
            if (tex == "") continue;

            Texture texture;
            texture.id = texIndex;
            texture.name = tex;

            textures.push_back(texture);

            texIndex += 1;
        }
    }

    if (d.contains("animation")) {
        isAnim = true;
        Resources::loadAnimation(d.getC<std::string>("animation", ""), this);
    }
}
void ModelFbx::updatePreRender(const unsigned int index, Shader& shader) {
}
void ModelFbx::updateRender(const unsigned int index, Shader& shader) {
    shader.setInt("NUM_LIGHTS", 0);
    if (index >= snorri::RENDER_LAYER_DIRECTIONAL_DEPTH && index <= 2) {
        if (index == snorri::RENDER_LAYER_POINT_DEPTH || index == 2) {
            Light* l = Light::getClosestPointLight(getPoint().getPosition());
            if (l != nullptr) {
                l->updateShadows(shader);
                //Log::console("model primitive found close light!");
                shader.setInt("NUM_LIGHTS", 1);
            }
        }
        updateBonesCheck(shader);
        shader.setMat4("M_MODEL", getTransformation());
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
bool ModelFbx::isValidLayer(const unsigned int index) {
    if (index >= 0 && index <= 2) {
        return true;
    } return false;
}

bool model_fbx_registered = []() {
    Object::registerComponent("model_fbx", [](Object* parent) {
        return std::make_shared<ModelFbx>(parent);
    });
    return true;
}();