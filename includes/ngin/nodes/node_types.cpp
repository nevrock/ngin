#include <ngin/node/node_graph.h>

#include <ngin/nodes/object.h>
#include <ngin/nodes/transform.h>
#include <ngin/nodes/pass.h>
#include <ngin/nodes/camera.h>
#include <ngin/nodes/shader.h>
#include <ngin/nodes/shader_drawer.h>
#include <ngin/nodes/model_drawer.h>

#include <ngin/nodes/gl/gl_depth_test.h>


bool pass_registered = []() {
    NodeGraph::registerNodeType("pass", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Pass>(name, dictionary);
    });
    return true;
}();

bool transform_registered = []() {
    NodeGraph::registerNodeType("transform", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Transform>(name, dictionary);
    });
    return true;
}();

bool object_registered = []() {
    NodeGraph::registerNodeType("object", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Object>(name, dictionary);
    });
    return true;
}();

bool camera_registered = []() {
    NodeGraph::registerNodeType("camera", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Camera>(name, dictionary);
    });
    return true;
}();

bool shader_registered = []() {
    NodeGraph::registerNodeType("shader", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<Shader>(name, dictionary);
    });
    return true;
}();

bool gl_depth_test_registered = []() {
    NodeGraph::registerNodeType("gl_depth_test", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlDepthTest>(name, dictionary);
    });
    return true;
}();

bool model_drawer_registered = []() {
    NodeGraph::registerNodeType("model_drawer", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<ModelDrawer>(name, dictionary);
    });
    return true;
}();

bool shader_drawer_registered = []() {
    NodeGraph::registerNodeType("shader_drawer", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<ShaderDrawer>(name, dictionary);
    });
    return true;
}();
