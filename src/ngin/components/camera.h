#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>
#include <ngin/game.h>

#include <ngin/drawer.h>    

#include <ngin/utils/mathutils.h>   

class Camera : public IDrawer {
public:
    Camera(const std::string name, const Lex& lex, IObject* parent)
        : IDrawer(name, lex, parent) {
        
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }

        speed_ = lex.getC<float>("speed", 0.1f);
    }
    ~Camera() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
    }

    void init() override {
        // Implementation of init method
        if (lex_.contains("zoom")) {
            zoom_ = lex_.getC<float>("zoom", 1.0);
        } else {
            zoom_ = 45.0;
        }
        if (lex_.contains("movement_speed")) {
            movementSpeed_ = lex_.getC<float>("movement_speed", 1.0);
        } else {
            movementSpeed_ = 2.5;
        }
    }

    void launch() override {
        // Implementation of launch method
    }

    void update() override {
        // Implementation of update method
        TransformData* transform = getTransform();

        float yaw = transform->getYaw();
        float pitch = transform->getPitch();

        float mouseX = Game::envget<float>("mouse.offsetX");
        float mouseY = Game::envget<float>("mouse.offsetY");
        float axesX = Game::envget<float>("axesX");
        float axesY = Game::envget<float>("axesY");

        glm::vec3 position = transform->getPosition();


        float yawN   = yaw + mouseX*0.1;
        float pitchN = pitch + mouseY*0.1;

        if (pitchN > 89.0f)
            pitchN = 89.0f;
        if (pitchN < -89.0f)
            pitchN = -89.0f;

        yawN = MathUtils::lerp(yaw, yawN, speed_);
        pitchN = MathUtils::lerp(pitch, pitchN, speed_);

        transform->setRotationFromYawPitch(yawN, pitchN);

        //transform->setRotationFromYawPitch(yaw, pitch);

        glm::vec3 front = getTransform()->getForward();
        glm::vec3 right = getTransform()->getRight();
        glm::vec3 up = getTransform()->getUp();

        float velocity = movementSpeed_ * Game::envget<float>("time.delta");

        position += front * axesY * velocity;
        position += right * axesX * velocity;

        transform->setPosition(position);
    }

    void updateLate() override {
        // Implementation of updateLate method
    }

    glm::mat4 getViewMatrix()
    {
        TransformData* transform = getTransform();
        glm::vec3 position = transform->getPosition();
        glm::vec3 front = transform->getForward();
        glm::vec3 up = transform->getUp();
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight)
    {
        glm::mat4 projection = glm::perspective(glm::radians(zoom_), screenWidth / screenHeight, 0.1f, 100.0f);
        return projection;
    }

    void prep(ShaderData& shader) override {
        int screenWidth = Game::envget<int>("screen.width");
        int screenHeight = Game::envget<int>("screen.height");

        glm::mat4 projection = getProjectionMatrix(screenWidth, screenHeight);
        glm::mat4 view = getViewMatrix();
        glm::mat4 inverseViewMatrix = glm::inverse(view);
        glm::mat4 inverseProjectionMatrix = glm::inverse(projection);

        shader.setMat4("view", view);
        //shader.setMat4("M_CAMERA_VIEW", view);
        shader.setMat4("projection", projection);
        Game::envset<glm::mat4>("projection", projection);
        //shader.setMat4("M_CAMERA_PROJECTION", projection);
        //shader.setMat4("M_CAMERA_I_VIEW", inverseViewMatrix);
        //shader.setMat4("M_CAMERA_I_PROJECTION", inverseProjectionMatrix);
        //shader.setVec3("CAMERA_POS", getTransform()->getPosition());
        shader.setVec3("viewPos", getTransform()->getPosition());
        shader.setVec3("camPos", getTransform()->getPosition());
        //shader.setFloat("CAMERA_NEAR_PLANE", 0.1f);
        //shader.setFloat("CAMERA_FAR_PLANE", 100.0f);

        //Log::console("Camera position: " + std::to_string(getTransform()->getPosition().x) + ", " + std::to_string(getTransform()->getPosition().y) + ", " + std::to_string(getTransform()->getPosition().z));
    }

    void draw(ShaderData& shader) override {
        // Implementation of draw method
    }

private:
    //float zoom_, pitch_, movementSpeed_, mouseSensitivity_;
    float zoom_, movementSpeed_;

    float speed_;
};

#endif // CAMERA_H
