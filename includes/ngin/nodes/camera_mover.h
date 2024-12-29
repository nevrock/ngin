#ifndef CAMERA_MOVER_H
#define CAMERA_MOVER_H

#include <ngin/data/transform_data.h>
#include <ngin/data/mover_data.h>
#include <ngin/node/node.h>
#include <ngin/collections/nevf.h>
#include <ngin/gl/window.h>

#include <ngin/game.h>

class CameraMover : public Node {
public:
    explicit CameraMover(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), moverData_(std::make_shared<MoverData>()), movementSpeed_(dictionary.getC<float>("movement_speed", 6.0f))       
    {
    }
    ~CameraMover() override = default;

    void setup() override {
        moverData_ = std::make_shared<MoverData>();

        // Subscribe to key press events
        Window::onKeyPressW.addListener([this](const Nevf& args) { onKeyPressW(args); });
        Window::onKeyPressS.addListener([this](const Nevf& args) { onKeyPressS(args); });
        Window::onKeyPressA.addListener([this](const Nevf& args) { onKeyPressA(args); });
        Window::onKeyPressD.addListener([this](const Nevf& args) { onKeyPressD(args); });
    }
    void update(std::string& pass) override {
        if (pass == "logic") {
            // read TransformData input, setup MoverData to pass in transform pass
            std::shared_ptr<NodePort> inputPortTransform = getInputPortByTypeAndData<TransformData>(pass);
            if (inputPortTransform) {
                transformData_ = inputPortTransform->getData<TransformData>();
                
                if (transformData_) {
                    float yaw = transformData_->getYaw();
                    float pitch = transformData_->getPitch();

                    float mouseX = Game::env<float>("mouse_x");
                    float mouseY = Game::env<float>("mouse_y");
                
                    yaw   += mouseX;
                    pitch += mouseY;

                    if (pitch > 89.0f)
                        pitch = 89.0f;
                    if (pitch < -89.0f)
                        pitch = -89.0f;

                    glm::quat currentRotation = transformData_->getRotation();
                    glm::quat targetRotation;
                    targetRotation = glm::rotate(targetRotation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
                    targetRotation = glm::rotate(targetRotation, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

                    glm::quat rotationDelta = glm::inverse(currentRotation) * targetRotation;
                    moverData_->setRotationDelta(rotationDelta);
                }
            } else {
                moverData_.reset();
                moverData_ = std::make_shared<MoverData>();
            }
        } else if (pass == "transform") {
            // set the transform type output port data to moverData_
            std::shared_ptr<NodePort> outputPortTransform = getOutputPortByType(pass);
            if (outputPortTransform) {
                outputPortTransform->setData(moverData_);
            }
        }
    }
    void clean(std::string& pass) override {
        if (pass == "logic") {
            glm::vec3 positionDelta = moverData_->getPositionDelta();
            positionDelta = glm::mix(positionDelta, glm::vec3(0.0f), Window::deltaTime * movementSpeed_);
            moverData_->setPositionDelta(positionDelta);
        }
    }

private:
    std::shared_ptr<MoverData> moverData_;
    std::shared_ptr<TransformData> transformData_;

    float movementSpeed_;

    float getVelocity() {
        return movementSpeed_*Window::deltaTime;
    }

    void onKeyPressW(const Nevf& args) {
        // Implement logic for W key press
        std::cout << "W key pressed" << std::endl;
        if (!transformData_) {
            return;
        }
        float velocity = getVelocity();
        moverData_->setPositionDelta(transformData_->getFrontDirection() * velocity);
    }

    void onKeyPressS(const Nevf& args) {
        // Implement logic for S key press
        std::cout << "S key pressed" << std::endl;
        if (!transformData_) {
            return;
        }
        float velocity = getVelocity();
        moverData_->setPositionDelta(-(transformData_->getFrontDirection() * velocity));
    }

    void onKeyPressA(const Nevf& args) {
        // Implement logic for A key press
        std::cout << "A key pressed" << std::endl;
        if (!transformData_) {
            return;
        }
        float velocity = getVelocity();
        moverData_->setPositionDelta(-(transformData_->getRightDirection() * velocity));
    }

    void onKeyPressD(const Nevf& args) {
        // Implement logic for D key press
        std::cout << "D key pressed" << std::endl;
        if (!transformData_) {
            return;
        }
        float velocity = getVelocity();
        moverData_->setPositionDelta(transformData_->getRightDirection() * velocity);
    }
};

#endif