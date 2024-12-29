#include <ngin/node/node_graph.h>

#include <ngin/nodes/object.h>
#include <ngin/nodes/transform.h>
#include <ngin/nodes/pass.h>
#include <ngin/nodes/camera.h>
#include <ngin/nodes/camera_mover.h>
#include <ngin/nodes/shader.h>
#include <ngin/nodes/shader_drawer.h>
#include <ngin/nodes/model_drawer.h>
#include <ngin/nodes/shader_attributes.h>
#include <ngin/nodes/light_directional.h>

#include <ngin/nodes/gl/gl_shadows_directional.h>
#include <ngin/nodes/gl/gl_shadows_point.h>
#include <ngin/nodes/gl/gl_bsdf.h>

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

bool camera_mover_registered = []() {
    NodeGraph::registerNodeType("camera_mover", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<CameraMover>(name, dictionary);
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

bool light_directional_registered = []() {
    NodeGraph::registerNodeType("light_directional", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<LightDirectional>(name, dictionary);
    });
    return true;
}();




bool gl_shadows_directional = []() {
    NodeGraph::registerNodeType("gl_shadows_directional", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlShadowsDirectional>(name, dictionary);
    });
    return true;
}();

bool gl_shadows_point = []() {
    NodeGraph::registerNodeType("gl_shadows_point", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlShadowsPoint>(name, dictionary);
    });
    return true;
}();

bool gl_bsdf = []() {
    NodeGraph::registerNodeType("gl_bsdf", [](const std::string& name, Nevf& dictionary) {
        return std::make_shared<GlBsdf>(name, dictionary);
    });
    return true;
}();
