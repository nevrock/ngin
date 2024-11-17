#ifndef SCENE_H
#define SCENE_H

#include <iostream> 
#include <vector>
#include <snorri/dict.h>
#include <snorri/resources.h>
#include <snorri/object.h>
#include <snorri/model_drawer.h>
#include <memory> // Include for smart pointers
#include <unordered_map>

class Scene {
public:
    std::unique_ptr<Object> origin;  // Use smart pointer for origin
    std::unordered_map<int, std::string> shaderNames;  // Store shader IDs and names

    Scene(const std::string& title) : originName(title) {
    }   

    void initScene() {

        Dict d;
        d.read(FileUtils::getResourcePath("snorri/resources.snorri").c_str());
        d.print();

        if (d.contains("shaders")) {
            for (const auto& pair : d.get<Dict>("shaders", nullptr)->data()) {
                int shaderId = std::any_cast<int>(pair.second);
                shaderNames[shaderId] = pair.first;
                Resources::loadShader(pair.first);
            }
        }
        if (d.contains("textures")) {
            for (const auto& pair : d.get<Dict>("textures", nullptr)->data()) {
                Resources::loadTexture(pair.first);
            }
        }
        if (d.contains("models")) {
            for (const auto& pair : d.get<Dict>("models", nullptr)->data()) {
                Resources::loadModel(pair.first);
            }
        }
        if (d.contains("models_primitive")) {
            for (const auto& pair : d.get<Dict>("models_primitive", nullptr)->data()) {
                Resources::loadModelPrimitive(pair.first);
            }
        }
        if (d.contains("fonts")) {
            for (const auto& pair : d.get<Dict>("fonts", nullptr)->data()) {
                Resources::loadFont(pair.first);
            }
        }

        ModelDrawer::init();
    }
    void build() {
        Dict d;
        d.read(FileUtils::getResourcePath("snorri/" + originName + ".snorri").c_str());
        d.print();
        origin = std::make_unique<Object>(d, 0, nullptr, false);
    }

    ~Scene() {}
    
    static bool isApplicationShuttingDown;

    void launch() {
        origin->launch();
    }
    void updateAnimation(float dt) {
        origin->updateAnimation(dt);
    }
    void update() {
        origin->update();
    }
    void updatePhysics(float dt) {
        origin->updatePhysics(dt);
    }
    void updatePreRender(const unsigned int index, Shader& shader) {
        origin->updatePreRender(index, shader);
    }
    void updateRender(const unsigned int index, Shader& shader) {
        //origin->updateRender(index, shader);
        ModelDrawer::render(index, shader);
        ModelDrawer::renderInstances(index, shader);
    }   

    void initDepth() {
        for (const auto& [shaderId, shaderName] : shaderNames) {
            Shader shader = Resources::getShader(shaderName);
            shader.use();
            shader.setInt("DIRECTIONAL_SHADOW_MAP", 0);
            shader.setInt("POINT_SHADOW_CUBE_MAP", 1);
        }
    }

    void render(const unsigned int index) {
        if (!shaderNames.empty() && shaderNames.count(index)) {
            Shader shader = Resources::getShader(shaderNames[index]);
            updateRender(index, shader);
        }
    }
    void preRender(const unsigned int index) {
        if (!shaderNames.empty() && shaderNames.count(index)) {
            Shader shader = Resources::getShader(shaderNames[index]);
            shader.use();
            updatePreRender(index, shader);
        }
    }

    void updateShadows(Light* light) {
         if (!shaderNames.empty()) {
            Shader shader = Resources::getShader(shaderNames.begin()->second);
            light->updateShadows(shader);
        }
    }

    int getShaderCount() const {
        return shaderNames.size();
    }

    void clear() {
        Resources::clear();
    }
        
private:
    std::string originName;
};

bool Scene::isApplicationShuttingDown = false;  // Define and initialize the static bool

#endif // SCENE_H
