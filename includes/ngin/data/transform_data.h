#ifndef TRANSFORM_DATA_H
#define TRANSFORM_DATA_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ngin/data/i_data.h>

class TransformData : public IData {
public:

  TransformData() : position_(0.0f), rotation_(1.0f, 0.0f, 0.0f, 0.0f), scale_(1.0f), parent_(1.0f) {} 
  TransformData(const Nevf& data) :  position_(0.0f), rotation_(1.0f, 0.0f, 0.0f, 0.0f), scale_(1.0f), parent_(1.0f) {
    if (data.contains("position")) {
      position_ = data.getVec("position", glm::vec3(0.0));
    }
    if (data.contains("rotation")) {
      rotation_ = data.getVec("rotation", glm::vec3(0.0));
    }
    if (data.contains("scale")) {
      scale_ = data.getVec("scale", glm::vec3(1.0));
    }
  }

  void execute() override {
    // cook this data so it is stable for remaining passes
  }

  std::string getName() override { return name_; } 

  // Position
  glm::vec3 getPosition() const { return position_; }
  void setPosition(const glm::vec3& position) { position_ = position; }
  glm::vec3 getWorldPosition() const {
    glm::vec4 localPosition(position_, 1.0f); // Create a vec4 with w = 1
    glm::vec4 worldPosition = parent_ * localPosition; 
    return glm::vec3(worldPosition); // Extract the vec3 part
  }

  glm::vec3 getForward() const {
      glm::vec3 forward(0.0f, 0.0f, -1.0f); // Assuming forward is -Z
      return glm::mat3_cast(rotation_) * forward; // Use mat3_cast
  }

  glm::vec3 getUp() const {
      glm::vec3 up(0.0f, 1.0f, 0.0f); // Assuming up is +Y
      return glm::mat3_cast(rotation_) * up; // Use mat3_cast
  }

  // Rotation
  glm::quat getRotation() const { return rotation_; }
  void setRotation(const glm::quat& rotation) { rotation_ = rotation; }

  // Scale
  glm::vec3 getScale() const { return scale_; }
  void setScale(const glm::vec3& scale) { scale_ = scale; }

  void setParentModel(const glm::mat4& parentModel) { parent_ = parentModel; }
  glm::mat4 getParentModel() const { return parent_; }

  glm::mat4 getWorldModelMatrix() const {
    return parent_ * getModelMatrix(); 
  }
  

  glm::mat4 getModelMatrix() const {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, position_);
      model = glm::rotate(model, glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::scale(model, scale_);

      return model;
  }
  glm::vec3 getFrontDirection() const {
      glm::vec3 front;
      front.x = sin(glm::radians(rotation_.y));
      front.y = sin(glm::radians(rotation_.x));
      front.z = cos(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));
      return glm::normalize(front);
  }
  glm::vec3 getUpDirection() const {
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming 'up' is always Y-axis aligned
      glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation_.x), glm::vec3(1, 0, 0));
      rot = glm::rotate(rot, glm::radians(rotation_.y), glm::vec3(0, 1, 0));
      rot = glm::rotate(rot, glm::radians(rotation_.z), glm::vec3(0, 0, 1));
      return glm::mat3(rot) * up;
  }


private:
  std::string name_;

  glm::vec3 position_;
  glm::quat rotation_;
  glm::vec3 scale_;

  // this gets set when the graph is cooked from the parent node
  glm::mat4 parent_;
};

#endif