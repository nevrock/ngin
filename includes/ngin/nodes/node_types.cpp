#include <ngin/node/node_graph.h>

#include <ngin/nodes/object.h>
#include <ngin/nodes/transform.h>
#include <ngin/nodes/pass.h>
#include <ngin/nodes/camera.h>
#include <ngin/nodes/shader.h>
#include <ngin/nodes/shader_drawer.h>
#include <ngin/nodes/model_drawer.h>
#include <ngin/nodes/shader_attributes.h>

#include <ngin/nodes/gl/gl_depth_test.h>
#include <ngin/nodes/gl/gl_g_buffer.h>
#include <ngin/nodes/gl/gl_bind_buffer.h>
#include <ngin/nodes/gl/gl_window.h>
#include <ngin/nodes/gl/gl_copy_buffer.h>

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


bool shader_attributes_registered = []() {
    NodeGraph::registerNodeType("shader_attributes", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<ShaderAttributes>(name, dictionary);
    });
    return true;
}();





bool gl_depth_test_registered = []() {
    NodeGraph::registerNodeType("gl_depth_test", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlDepthTest>(name, dictionary);
    });
    return true;
}();

bool gl_g_buffer_registered = []() {
    NodeGraph::registerNodeType("gl_g_buffer", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlGBuffer>(name, dictionary);
    });
    return true;
}();

bool gl_bind_buffer_registered = []() {
    NodeGraph::registerNodeType("gl_bind_buffer", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlBindBuffer>(name, dictionary);
    });
    return true;
}();

bool gl_window_registered = []() {
    NodeGraph::registerNodeType("gl_window", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlWindow>(name, dictionary);
    });
    return true;
}();

bool gl_copy_buffer_registered = []() {
    NodeGraph::registerNodeType("gl_copy_buffer", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlCopyBuffer>(name, dictionary);
    });
    return true;
}();