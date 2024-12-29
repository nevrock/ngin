#ifndef SHADER_ATTRIBUTES_H
#define SHADER_ATTRIBUTES_H

#include <ngin/nodes/shader.h>
#include <ngin/node/node.h>
#include <ngin/data/render_data.h>
#include <ngin/collections/nevf.h>

class ShaderAttributes : public Node {
public:
    explicit ShaderAttributes(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), 
          attributesStart_(dictionary.get<Nevf>("attributes_start", nullptr)),
          attributesUpdate_(dictionary.get<Nevf>("attributes_update", nullptr))
    {
    }

    void start(std::string& pass) override {
        Node::start(pass); // Call base class setup

        auto inputPort = getInputPortByType(pass);
        if (inputPort) {
            std::shared_ptr<RenderData> renderData = inputPort->getData<RenderData>();
            if (renderData) {
                ShaderData& shader = renderData->getShader();
                for (const auto& key : attributesStart_->keys()) {
                    const auto& value = (*attributesStart_)[key];
                    setShaderAttribute(key, value, shader);
                }
            }
        }
    }

    void update(std::string& pass) override {
        Node::update(pass); // Call base class update

        auto inputPort = getInputPortByType(pass);
        if (inputPort) {
            std::shared_ptr<RenderData> renderData = inputPort->getData<RenderData>();
            if (renderData) {
                ShaderData& shader = renderData->getShader();
                for (const auto& key : attributesUpdate_->keys()) {
                    const auto& value = (*attributesUpdate_)[key];
                    setShaderAttribute(key, value, shader);
                }
            }
        }
    }

private:
    void setShaderAttribute(const std::string& key, const std::any& value, ShaderData& shader) {
        if (value.type() == typeid(int)) {
            shader.setInt(key, std::any_cast<int>(value));
        } else if (value.type() == typeid(float)) {
            shader.setFloat(key, std::any_cast<float>(value));
        } else if (value.type() == typeid(bool)) {
            shader.setBool(key, std::any_cast<bool>(value));
        } else if ((value.type() == typeid(std::vector<int>)) || (value.type() == typeid(std::vector<float>))) {
            if (value.type() == typeid(std::vector<int>)) {
                const auto vec = std::any_cast<std::vector<int>>(value);
                if (vec.size() == 2) {
                    shader.setVec2(key, glm::vec2(vec[0], vec[1]));
                } else if (vec.size() == 3) {
                    shader.setVec3(key, glm::vec3(vec[0], vec[1], vec[2]));
                } else if (vec.size() == 4) {
                    shader.setVec4(key, glm::vec4(vec[0], vec[1], vec[2], vec[3]));
                }
            } else if (value.type() == typeid(std::vector<float>)) {
                const auto vec = std::any_cast<std::vector<float>>(value);
                if (vec.size() == 2) {
                    shader.setVec2(key, glm::vec2(vec[0], vec[1]));
                } else if (vec.size() == 3) {
                    shader.setVec3(key, glm::vec3(vec[0], vec[1], vec[2]));
                } else if (vec.size() == 4) {
                    shader.setVec4(key, glm::vec4(vec[0], vec[1], vec[2], vec[3]));
                }
            }
        }
    }

    Nevf* attributesStart_;
    Nevf* attributesUpdate_;
};

#endif
