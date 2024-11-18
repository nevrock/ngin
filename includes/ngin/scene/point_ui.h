#ifndef POINT_UI_H
#define POINT_UI_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ngin/scene/point.h>
#include <ngin/constants.h>

class PointUi : public Point {
public:

    // Default constructor
    PointUi() : Point() {}

    // Parameterized constructor
    PointUi(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
        : Point(position, rotation, scale) {}


    glm::mat4 create2DTransformMatrix(const glm::vec3& translation, float rotationAngle, const glm::vec2& scale) const {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));

        glm::mat4 transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        return transformMatrix;
    }
    glm::mat4 getUiMatrix(glm::vec2 pos) const {
        float xScale = (size_.x)/(ngin::SCREEN_WIDTH/2.0);
        float yScale = (size_.y)/(ngin::SCREEN_HEIGHT/2.0);
        float xPos = (pos.x - (ngin::SCREEN_WIDTH/2.0))/(ngin::SCREEN_WIDTH/2.0);
        float yPos = (pos.y - (ngin::SCREEN_HEIGHT/2.0))/(ngin::SCREEN_HEIGHT/2.0);
        return create2DTransformMatrix(glm::vec3(xPos,yPos,0), 0, 
                glm::vec2(xScale, yScale));
    }

    glm::mat4 getModelMatrix() const override {
        return create2DTransformMatrix(position_, 0.0, scale_);
    }

    void setSize(glm::vec2 size) {
        size_ = size;
    }
    void setAnchor(glm::vec2 anchor) {
        anchor_ = anchor;
    }

private:
    glm::vec2 size_;
    glm::vec2 anchor_;
};

#endif // POINT_UI_H
