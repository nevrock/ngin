#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class Point {
public:

    // Default constructor
    Point() : position_(0.0f), rotation_(0.0f), scale_(1.0f) {}

    // Parameterized constructor
    Point(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
        : position_(position), rotation_(rotation), scale_(scale) {}

    // Getters
    virtual glm::vec3 getPosition() const {
        return position_;
    }
    virtual glm::vec3 getRotation() const {
        return rotation_;
    }
    virtual glm::vec3 getScale() const {
        return scale_;
    }

    // Setters
    virtual void setPosition(const glm::vec3& position) {
        position_ = position;
    }
    virtual void setRotation(const glm::vec3& rotation) {
        rotation_ = rotation;
    }
    virtual void setScale(const glm::vec3& scale) {
        scale_ = scale;
    }

    // Method to get the model matrix
    virtual glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position_);
        model = glm::rotate(model, glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale_);

        return model;
    }
    virtual glm::vec3 getFrontDirection() const {
        glm::vec3 front;
        front.x = sin(glm::radians(rotation_.y));
        front.y = sin(glm::radians(rotation_.x));
        front.z = cos(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));
        return glm::normalize(front);
    }
    virtual glm::vec3 getUpDirection() const {
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming 'up' is always Y-axis aligned
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation_.x), glm::vec3(1, 0, 0));
        rot = glm::rotate(rot, glm::radians(rotation_.y), glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, glm::radians(rotation_.z), glm::vec3(0, 0, 1));
        return glm::mat3(rot) * up;
    }

protected:
    glm::vec3 position_;
    glm::vec3 rotation_;
    glm::vec3 scale_;
};

#endif // POINT_H
