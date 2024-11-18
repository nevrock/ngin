#include "model_sdf.h"
#include <iostream>
#include <vector>
#include "../light/light.h"
#include <ngin/constants.h>
#include <ngin/surface_data.h>
#include <ngin/surface_sdf.h>

ModelSdf::ModelSdf(Object* parent) : Model(parent), surfacesBuffer(0), currentBufferSize(0) {
}

ModelSdf::~ModelSdf() {
    if (surfacesBuffer != 0) {
        glDeleteBuffers(1, &surfacesBuffer);
    }
}

void ModelSdf::initializeBuffers() {
    glGenBuffers(1, &surfacesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, surfacesBuffer);
    // Initial buffer size set to zero since no data at initialization
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, surfacesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

unsigned int ModelSdf::updateBuffers() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, surfacesBuffer);

    Object* myObject = dynamic_cast<Object*>(parent);
    if (!myObject) {
        std::cerr << "Failed to dynamic cast parent to Object*.\n";
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return -1;
    }

    auto sdfs = myObject->getComponentsInChildren<SurfaceSdf>();
    unsigned int numSurfaces = sdfs.size();
    size_t requiredBufferSize = sizeof(SurfaceData) * numSurfaces;

    if (requiredBufferSize != currentBufferSize) {
        currentBufferSize = requiredBufferSize;
        glBufferData(GL_SHADER_STORAGE_BUFFER, currentBufferSize, nullptr, GL_DYNAMIC_DRAW);
    }

    SurfaceData* surfaceData = static_cast<SurfaceData*>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        0,
        currentBufferSize,
        GL_MAP_WRITE_BIT
    ));

    if (surfaceData) {
        //Log::console("model sdf prepared to map data!");
        for (unsigned int i = 0; i < numSurfaces; ++i) {
            surfaceData[i] = sdfs[i]->getSurfaceData();
            //Log::console("model sdf added data - ");
            //Log::console(surfaceData[i].position);
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    } else {
        std::cerr << "Failed to map surface data buffer for writing.\n";
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return numSurfaces;
}

void ModelSdf::loadFromDict(const Dict& d) {
    fileName = "cube";
    data = Resources::getModel(std::string("cube"));

    isInstance = d.getC<bool>("is_instance", false);
    isCull = d.getC<bool>("is_cull", false);
    layer = d.getC<int>("layer", 0);
    
    initializeBuffers();
}

void ModelSdf::updatePreRender(const unsigned int index, Shader& shader) {
    // Implementation as necessary
}

void ModelSdf::updateRender(const unsigned int index, Shader& shader) {
    if (index == 3 || index == 0) {
        glCullFace(GL_FRONT);

        unsigned int numSurfaces = updateBuffers();
        shader.setInt("NUM_SURFACES", numSurfaces);
        shader.setBool("IS_SDF", true);
        shader.setInt("NUM_LIGHTS", 0);
        if (index == snorri::RENDER_LAYER_POINT_DEPTH || index == 2) {
            glm::vec3 pos = getPoint().getPosition();
            Light* l = Light::getClosestPointLight(pos);
            if (l != nullptr) {
                l->updateShadows(shader);
                shader.setInt("NUM_LIGHTS", 1);
            }
        }
        shader.setMat4("M_MODEL", getPoint().getModelMatrix());
        shader.setFloat("IS_ANIMATION", false);
        data->render();
        shader.setBool("IS_SDF", false);

        glCullFace(GL_BACK);
    }
}

bool ModelSdf::isValidLayer(const unsigned int index) {
    return index == 3 || index == 0;
}

bool model_sdf_registered = []() {
    Object::registerComponent("model_sdf", [](Object* parent) {
        return std::make_shared<ModelSdf>(parent);
    });
    return true;
}();
