// model_drawer.h

#ifndef DRAWER_H
#define DRAWER_H

#include <map>
#include <string>
#include <vector>
#include <functional> // Include for std::reference_wrapper

#include <ngin/scene/i_drawer.h> // Include IDrawer
#include <ngin/resources.h> // Include IDrawer

class Drawer {
public:

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

};

#endif
