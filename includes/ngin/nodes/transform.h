#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>
#include <tuple>

#include <ngin/node/node.h>
#include <ngin/data/transform_data.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp> // Include this header for glm::to_string

// Object class inheriting from Node
class Transform : public Node {
public:
    Transform(const std::string& name, Nevf& data) : Node(name, data) {
        transform_ = std::make_shared<TransformData>(data);
    }
    void execute(std::string& pass) override {
        retrieveInputData(pass);
        update(pass);
    }
    void update(std::string& pass) override {
        Node::update(pass);

        std::shared_ptr<NodePort> inputPort = getInputPortByType(pass);
        if (inputPort) {
            std::shared_ptr<TransformData> parentData = inputPort->getData<TransformData>();
            if (parentData) {
                transform_->setParentModel(parentData->getWorldModelMatrix());
            } else {
                transform_->setParentModel(glm::mat4(1.0f));
            }
        } else {
            transform_->setParentModel(glm::mat4(1.0f));
        }
        transform_->execute();

        setOutputData(pass, transform_);
    }

    void setup() override {
        Node::setup();
    }

    glm::vec3 getPosition() const { return transform_->getPosition(); }
    void setPosition(const glm::vec3& position) { transform_->setPosition(position); }

    glm::quat getRotation() const { return transform_->getRotation(); }
    void setRotation(const glm::quat& rotation) { transform_->setRotation(rotation); }

    glm::vec3 getScale() const { return transform_->getScale(); }
    void setScale(const glm::vec3& scale) { transform_->setScale(scale); }

private:
    std::shared_ptr<TransformData> transform_;
};

#endif // TRANSFORM_Hr