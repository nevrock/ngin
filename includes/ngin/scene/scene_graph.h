#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <string>
#include <memory>

#include <ngin/node/node_graph.h>
#include <ngin/scene/object.h>

// SceneGraph class inheriting from NodeGraph
class SceneGraph : public NodeGraph {
public:
    SceneGraph(const std::string& filepath) : NodeGraph(filepath) {
        buildObjects();
        //buildConnections();
    }

    // Create an Object in the SceneGraph
    std::shared_ptr<Object> createObject(const std::string& name, Nevf& data) {
        auto object = std::make_shared<Object>(name, data);
        addNode(object);
        object->setGetNodeByNameFunc([this](const std::string& name) {
            return getNode(name); 
        });
        return object;
    }
    // Get an Object from the SceneGraph
    std::shared_ptr<Object> getObject(const std::string& name) const {
        auto node = getNode(name); // Use NodeGraph's getNode method
        return std::dynamic_pointer_cast<Object>(node); // Cast to Object
    }

private:
    void buildObjects() {
        Nevf* objs = data_.get<Nevf>("objects", nullptr);
        if (objs != nullptr) {
             for (const auto& key : objs->keys()) {
                std::cout << "scene graph building object: " << key << std::endl;
                createObject(key, *objs->get<Nevf>(key, nullptr));
            }
        }
    }
    void buildConnections() {

    }
};

#endif // SCENE_GRAPH_H
