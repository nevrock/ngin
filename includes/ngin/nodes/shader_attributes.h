#ifndef SHADER_ATTRIBUTES_H
#define SHADER_ATTRIBUTES_H

#include <ngin/nodes/shader.h>
#include <ngin/node/node.h>
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
            shader_ = inputPort->getData<ShaderData>();
        }

        if (shader_ && attributesStart_) {
            for (const auto& key : attributesStart_->keys()) {
                const auto& value = (*attributesStart_)[key];
                setShaderAttribute(key, value);
            }
        }
    }

    void update(std::string& pass) override {
        Node::update(pass); // Call base class update

        auto inputPort = getInputPortByType(pass);
        if (inputPort) {
            shader_ = inputPort->getData<ShaderData>();
        }

        if (shader_ && attributesUpdate_) {
            for (const auto& key : attributesUpdate_->keys()) {
                const auto& value = (*attributesUpdate_)[key];
                setShaderAttribute(key, value);
            }
        }
    }

private:
    void setShaderAttribute(const std::string& key, const std::any& value) {
        if (value.type() == typeid(int)) {
            shader_->setInt(key, std::any_cast<int>(value));
        } else if (value.type() == typeid(float)) {
            shader_->setFloat(key, std::any_cast<float>(value));
        } else if (value.type() == typeid(bool)) {
            shader_->setBool(key, std::any_cast<bool>(value));
        } else if ((value.type() == typeid(std::vector<int>)) || (value.type() == typeid(std::vector<float>))) {
            if (value.type() == typeid(std::vector<int>)) {
                const auto vec = std::any_cast<std::vector<int>>(value);
                if (vec.size() == 2) {
                    shader_->setVec2(key, glm::vec2(vec[0], vec[1]));
                } else if (vec.size() == 3) {
                    shader_->setVec3(key, glm::vec3(vec[0], vec[1], vec[2]));
                } else if (vec.size() == 4) {
                    shader_->setVec4(key, glm::vec4(vec[0], vec[1], vec[2], vec[3]));
                }
            } else if (value.type() == typeid(std::vector<float>)) {
                const auto vec = std::any_cast<std::vector<float>>(value);
                if (vec.size() == 2) {
                    shader_->setVec2(key, glm::vec2(vec[0], vec[1]));
                } else if (vec.size() == 3) {
                    shader_->setVec3(key, glm::vec3(vec[0], vec[1], vec[2]));
                } else if (vec.size() == 4) {
                    shader_->setVec4(key, glm::vec4(vec[0], vec[1], vec[2], vec[3]));
                }
            }
        }
    }

    Nevf* attributesStart_;
    Nevf* attributesUpdate_;
    std::shared_ptr<ShaderData> shader_;
};

#endif
