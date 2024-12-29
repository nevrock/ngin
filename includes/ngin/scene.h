#ifndef SCENE_H
#define SCENE_H

#include <ngin/event.h>
#include <ngin/node/node_graph.h>
#include <memory>
#include <functional>
#include <iostream>

class Scene {
public:
    // Constructor
    Scene() = default;

    // Deleted copy constructor and assignment operator
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Getter for the current scene name
    std::string& getCurrent() {
        return current_;
    }

    // Load a scene
    void load(const char* s) {
        graph_ = std::make_unique<NodeGraph>(s);
        graph_->build();
        
        current_ = std::string(s);
        eventState_.trigger();
    }
    void executePass(const std::string& passName) {
        graph_->executePass(passName);
    }
    void cleanPass(const std::string& passName) {
        graph_->cleanPass(passName);
    }
    void executePasses() {
        graph_->execute();
    }
    void cleanPasses() {
        graph_->clean();
    }
    // Add a listener for graph events
    void addGraphListener(std::function<void(const Nevf&)> listener) {
        eventState_.addListener(listener);
    }

private:
    // Instance-level members
    std::string current_;
    Event eventState_;
    
    std::unique_ptr<NodeGraph> graph_;
};

#endif // SCENE_H
