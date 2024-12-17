// model_drawer.h

#ifndef DRAWER_H
#define DRAWER_H

#include <map>
#include <string>
#include <vector>

#include <ngin/nodes/i_drawer.h> // Include IDrawer

class Drawer {
public:
    std::vector<std::shared_ptr<IDrawer>> getDrawers(const std::string& key) const {
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            return it->second; // Return the vector of drawers
        } else {
            return {}; // Return an empty vector if the key is not found
        }
    }

    static void registerDrawer(const std::string& key, std::shared_ptr<IDrawer> drawer) {
        drawers_[key].push_back(drawer); 
    }

    static void unregisterDrawer(const std::string& key, IDrawer* drawer) {
        auto& drawers = drawers_[key];
        drawers.erase(std::remove_if(drawers.begin(), drawers.end(),
                                     [drawer](const std::shared_ptr<IDrawer>& d) { return d.get() == drawer; }), 
                      drawers.end());
        if (drawers.empty()) {
            drawers_.erase(key);
        }
    }

    static void render(const std::string& key, ShaderData& shader) {
        auto it = drawers_.find(key);
        if (it != drawers_.end()) {
            for (const auto& drawer : it->second) {
                drawer->render(shader); // Call the render method on each drawer
            }
        }
    }

private:
    static std::map<std::string, std::vector<std::shared_ptr<IDrawer>>> drawers_; 
};

#endif
