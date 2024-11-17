#include "model_ui.h"
#include <nev/constants.h>
#include <iostream>

ModelUi::ModelUi(Object* parent) : Model(parent) {}

ModelUi::~ModelUi() {
}

void ModelUi::loadFromDict(const Dict& d) {

    fileName = std::string("quad");
    data = Resources::getModel("quad");

    isInstance = d.getC<bool>("is_instance", false);
    isCull = d.getC<bool>("is_cull", false);
    layer = d.getC<int>("layer", 0);
    isRenderChildren = d.getC<bool>("is_render_children", false);

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
glm::mat4 ModelUi::getTransformation() {
    glm::mat4 worldTransform = parent->getWorldMatrix(false);
    //Log::console("model ui world transform: " + getParent()->getName());
    //Log::console(worldTransform);

    //Log::console("model ui world position before transform: ");
    //Log::console(parent->getPoint().getPosition());

    glm::vec4 position = worldTransform * glm::vec4(parent->getPoint().getPosition(), 1); // Change order here
    //Log::console("model ui world position after transform: ");
    //Log::console(glm::vec3(position.x, position.y, position.z));

    glm::mat4 matrixOut = parent->getPointUi().getUiMatrix(glm::vec2(position.x, position.y));
    return matrixOut;
}
void ModelUi::updatePreRender(const unsigned int index, Shader& shader) {
    if (index == nev::RENDER_LAYER_UI) {
        //shader.setMat4("screenMat", createScreenToNDCMatrix(nev::SCREEN_WIDTH, nev::SCREEN_HEIGHT));
    }
}
void ModelUi::updateRender(const unsigned int index, Shader& shader) {
    if (index == nev::RENDER_LAYER_UI) {
        shader.setInt("NUM_LIGHTS", 0);

        shader.setMat4("M_MODEL", getTransformation());

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

        //Log::console("model ui render!");

        data->render();

        if (isRenderChildren) {
            Object* myObject = dynamic_cast<Object*>(parent);
            std::vector<std::shared_ptr<Model>> modelChildren = myObject->getComponentsInChildren<Model>();
            for (auto& model : modelChildren)
            {
                if (model->isCull)
                    model->updateRender(index, shader);
            }
        }
    }
}
bool ModelUi::isValidLayer(const unsigned int index) {
    if (index == nev::RENDER_LAYER_UI) {
        return true;
    } return false;
}

bool model_ui_registered = []() {
    Object::registerComponent("model_ui", [](Object* parent) {
        return std::make_shared<ModelUi>(parent);
    });
    return true;
}();
