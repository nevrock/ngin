// model_drawer.h

#ifndef DRAWER_H
#define DRAWER_H

#include <map>
#include <string>
#include <vector>
#include <functional> // Include for std::reference_wrapper

#include <ngin/scene/i_drawer.h> // Include IDrawer
#include <ngin/resources.h> // Include IDrawer

#include <ngin/data/instance_data.h> // Include InstanceData

class Drawer {
public:


    static void init() {
        glGenBuffers(1, &instanceBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer_);
        glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
    }

    std::vector<std::reference_wrapper<IDrawer>> getDrawers(const std::string& key) const {
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            return it->second; // Return the vector of drawers
        } else {
            return {}; // Return an empty vector if the key is not found
        }
    }

    static void registerDrawer(const std::string& key, IDrawer& drawer) {
        Log::console("Registering drawer: " + drawer.getName() + " for key: " + key, 1);  
        drawers_[key].push_back(drawer); 
    }
    static void unregisterDrawer(const std::string& key, IDrawer& drawer) {
        auto& drawers = drawers_[key];
        drawers.erase(std::remove_if(drawers.begin(), drawers.end(), [&drawer](std::reference_wrapper<IDrawer> d) {
            return &d.get() == &drawer;
        }), drawers.end());
        if (drawers.empty()) {
            drawers_.erase(key);
        }
    }

    static void registerInstance(const std::string& key, const std::string& mesh, IDrawer& instance) {
        Log::console("Registering instance: " + instance.getName() + " for key: " + key + " mesh: " + mesh, 1);  
        instances_[key][mesh].push_back(instance); 
    }
    static void unregisterInstance(const std::string& key, const std::string& mesh, IDrawer& instance) {
        auto& instances = instances_[key][mesh];
        instances.erase(std::remove_if(instances.begin(), instances.end(), [&instance](std::reference_wrapper<IDrawer> i) {
            return &i.get() == &instance;
        }), instances.end());
        if (instances.empty()) {
            instances_[key].erase(mesh);
        }
    }

    static void prep(const std::string& key) {
        ShaderData& shader = Resources::getShaderData(key);
        shader.use();
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            for (const auto& drawer : it->second) {
                //Log::console("Rendering drawer: " + drawer.getName());
                drawer.get().prep(shader); // Call the render method on each drawer
            }
        } else {
            //Log::console("No drawers found for key: " + key);
        }
    }
    static void prep(const std::string& key, ShaderData& data) {
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            for (const auto& drawer : it->second) {
                //Log::console("Rendering drawer: " + drawer.getName());
                drawer.get().prep(data); // Call the render method on each drawer
            }
        } else {
            //Log::console("No drawers found for key: " + key);
        }
    }
    static void draw(const std::string& key) {
        ShaderData& shader = Resources::getShaderData(key);
        shader.use();
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            for (const auto& drawer : it->second) {
                //Log::console("Rendering drawer: " + drawer.getName());
                drawer.get().draw(shader); // Call the render method on each drawer
            }
        } else {
            //Log::console("No drawers found for key: " + key);
        }
    }
    static void draw(const std::string& key, ShaderData& data) {
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            for (const auto& drawer : it->second) {
                //Log::console("Rendering drawer: " + drawer.getName());
                drawer.get().draw(data); // Call the render method on each drawer
            }
        } else {
            //Log::console("No drawers found for key: " + key);
        }
    }
    static void drawInstances(const std::string& key) {
        ShaderData& shader = Resources::getShaderData(key);
        shader.use();

        shader.setBool("IS_INSTANCE", true);

        for (const auto& [meshKey, instanceList] : instances_[key]) {

            std::vector<InstanceData> instanceDataArray;
            MeshData& meshData = Resources::getMeshData(meshKey);

            unsigned int tex = 0;

            for (const auto& instance : instanceList) {
                // Perform operations with each instance
                InstanceData idata = instance.get().getInstanceData(); // Example: Call draw method on each instance
                instanceDataArray.push_back(idata);

                if (tex == 0) {
                    tex = instance.get().getTextureId();
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer_);
            glBufferData(GL_ARRAY_BUFFER, instanceDataArray.size() * sizeof(InstanceData), instanceDataArray.data(), GL_DYNAMIC_DRAW);
       
            if (tex != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tex);
            }

            unsigned int VAO = meshData.VAO;
            glBindVertexArray(VAO);

            // Setup matrix attributes assuming locations 6-9 for matrix, 10 for UV, 11 for color
            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(6 + i);
                glVertexAttribPointer(6 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, modelMatrix) + sizeof(glm::vec4) * i));
                glVertexAttribDivisor(6 + i, 1);
            }

            // Draw all instances
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(meshData.indices.size()), GL_UNSIGNED_INT, 0, instanceDataArray.size());

            glBindVertexArray(0);  // Unbind VAO to avoid side effects
        
        }

        shader.setBool("IS_INSTANCE", false);
    }
    static bool raycastGui(glm::vec2 pos) {
        auto it = drawers_.find("gui");
        if (it != drawers_.end()) {
            auto& drawerList = it->second;
            for (auto rit = drawerList.rbegin(); rit != drawerList.rend(); ++rit) {
                bool isRaycast = rit->get().raycast(pos); // Call raycast on each drawer
                if (isRaycast) {
                    Log::console("Raycast hit: " + rit->get().getName());
                    return true; // Stop if a hit is found
                }
            }
        }
        return false;
    }

private:

    inline static std::map<std::string, std::vector<std::reference_wrapper<IDrawer>>> drawers_; 
    inline static std::map<std::string, std::map<std::string, std::vector<std::reference_wrapper<IDrawer>>>> instances_;

    inline static unsigned int instanceBuffer_; // Buffer for instance data

};

#endif
