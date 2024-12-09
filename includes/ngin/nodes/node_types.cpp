#include <ngin/node/node_graph.h>

#include <ngin/nodes/object.h>
#include <ngin/nodes/transform.h>
#include <ngin/nodes/pass.h>
#include <ngin/nodes/camera.h>


bool pass_registered = []() {
    NodeGraph::registerNodeType("pass", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Pass>(name, dictionary);
    });
    return true;
}();

bool obj_registered = []() {
    NodeGraph::registerNodeType("object", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Object>(name, dictionary);
    });
    return true;
}();

bool transform_registered = []() {
    NodeGraph::registerNodeType("transform", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Transform>(name, dictionary);
    });
    return true;
}();

bool camera_registered = []() {
    NodeGraph::registerNodeType("camera", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Camera>(name, dictionary);
    });
    return true;
}();