// model_drawer.cpp

#include <ngin/model_drawer.h>
#include <ngin/log.h>
#include <ngin/model.h>

using namespace snorri;

#include <GLFW/glfw3.h>

struct InstanceData {
    glm::mat4 modelMatrix;
    glm::vec2 uvOffset;
    glm::vec3 color;
};

std::map<unsigned int, DrawerData> ModelDrawer::data; // Definition
unsigned int ModelDrawer::buffer = 0;

void ModelDrawer::init() {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT_INSTANCES * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
}

void ModelDrawer::addModel(const std::string& key, unsigned int layer, Model* model) {
    Log::console("model drawer add model: " + key + ", layer - " + std::to_string(layer));
    data[layer].modelsLoadedByLayer[key].push_back(model);
    std::sort(data[layer].modelsLoadedByLayer[key].begin(), data[layer].modelsLoadedByLayer[key].end(),
        [](Model* a, Model* b) {
            return a->getSortIndex() > b->getSortIndex();
        });
}
bool ModelDrawer::removeModel(const std::string& key, unsigned int layer, Model* model) {
    Log::console("model drawer remove model: " + key);
    auto it = data.find(layer);
    if (it != data.end()) {
        auto& modelsLoadedByLayer = it->second.modelsLoadedByLayer[key];
        auto modelIt = std::remove(modelsLoadedByLayer.begin(), modelsLoadedByLayer.end(), model);
        if (modelIt != modelsLoadedByLayer.end()) {
            modelsLoadedByLayer.erase(modelIt, modelsLoadedByLayer.end()); // Erase the removed elements
            return true;
        }
    }
    return false;
}


void ModelDrawer::render(const unsigned int index, Shader& shader) {
    // Sort the modelsLoadedByLayer vector by getSortIndex() after adding a new model
    shader.setBool("IS_INSTANCE", false);
    for (auto& entry : data) {
        for (auto& layerEntry : entry.second.modelsLoadedByLayer) {
            for (Model* model : layerEntry.second) {
                if (model && !model->isInstance) {
                    model->updateRender(index, shader);
                }
            }
        }
    }
}
void ModelDrawer::renderInstances(const unsigned int index, Shader& shader) {
    shader.setBool("IS_INSTANCE", true);
    shader.setInt("NUM_LIGHTS", 0);
    shader.setFloat("IS_ANIMATION", false);

    int k = 0;
    for (auto& entry : data) {
        for (auto& layerEntry : entry.second.modelsLoadedByLayer) {
            if (layerEntry.second.empty()) continue;

            std::vector<InstanceData> instanceDataArray;

            int i = 0;
            for (Model* model : layerEntry.second) {
                if (!model->isInstance) {
                    continue;
                }

                if (!model->isValidLayer(index)) {
                    continue;
                }

                InstanceData instanceData;
                instanceData.modelMatrix = model->getTransformation();
                instanceData.uvOffset = model->getUvOffset();
                instanceData.color = model->getColor();
                instanceDataArray.push_back(instanceData);

                i++;
            }

            if (instanceDataArray.empty()) continue;

            ModelData* modelData = Resources::getModel(layerEntry.first);
            if (modelData == nullptr)
                continue;

            // Update buffer with the latest instance data
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, instanceDataArray.size() * sizeof(InstanceData), instanceDataArray.data(), GL_DYNAMIC_DRAW);

            // Render each mesh of the model
            for (auto& meshPtr : modelData->meshes) {

                if (index >= 2) {
                    int texIndex = 2;  
                    for (int i = 0; i < meshPtr->textures.size(); i++) {
                        Texture2D tex = Resources::getTexture(meshPtr->textures[i].name);
                        GLenum textureUnit = GL_TEXTURE0 + (texIndex + i);  // Calculate texture unit dynamically
                        glActiveTexture(textureUnit);
                        tex.bind();
                        shader.setInt("TEX_DIFFUSE", texIndex + i);
                    }
                }

                unsigned int VAO = meshPtr->VAO;
                glBindVertexArray(VAO);

                // Setup matrix attributes assuming locations 6-9 for matrix, 10 for UV, 11 for color
                for (int i = 0; i < 4; i++) {
                    glEnableVertexAttribArray(6 + i);
                    glVertexAttribPointer(6 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, modelMatrix) + sizeof(glm::vec4) * i));
                    glVertexAttribDivisor(6 + i, 1);
                }

                glEnableVertexAttribArray(10);
                glVertexAttribPointer(10, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, uvOffset));
                glVertexAttribDivisor(10, 1);

                glEnableVertexAttribArray(11);
                glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, color));
                glVertexAttribDivisor(11, 1);

                // Draw all instances
                glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(meshPtr->indices.size()), GL_UNSIGNED_INT, 0, instanceDataArray.size());

                glBindVertexArray(0);  // Unbind VAO to avoid side effects
            }

            k++;
        }
    }
    shader.setBool("IS_INSTANCE", false);
}
