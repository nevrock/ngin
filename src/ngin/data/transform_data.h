#ifndef TRANSFORM_DATA_H
#define TRANSFORM_DATA_H

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ngin/log.h>
#include <ngin/data/i_data.h>

class TransformData : public IData {
public:

  TransformData() : position_(0.0f), rotation_(1.0f, 0.0f, 0.0f, 0.0f), scale_(1.0f), parent_(1.0f) {} 
  TransformData(const Lex& data) :  position_(0.0f), rotation_(1.0f, 0.0f, 0.0f, 0.0f), scale_(1.0f), parent_(1.0f) {
    parent_ = glm::mat4(1.0f);
    if (data.contains("position")) {
      position_ = data.getVec("position", glm::vec3(0.0));
    }
    if (data.contains("rotation")) {
      glm::vec3 rotationVec = data.getVec("rotation", glm::vec3(0.0f));
      rotation_ = glm::quat(glm::vec3(glm::radians(rotationVec.x), glm::radians(rotationVec.y), glm::radians(rotationVec.z)));
      //rotation_ = glm::quat(rotationVec.w, rotationVec.x, rotationVec.y, rotationVec.z);
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
  glm::vec3 getPosition() const { 
    return position_; 
  }
  void setPosition(const glm::vec3& position) { 
    position_ = position; 
  }
  glm::vec3 getWorldPosition() const {
    glm::vec4 localPosition(position_, 1.0f); // Create a vec4 with w = 1
    glm::vec4 worldPosition = parent_ * localPosition; 
    return glm::vec3(worldPosition); // Extract the vec3 part
  }

  glm::vec3 getForward() const {
      glm::vec3 forward(0.0f, 1.0f, 0.0f);
      return glm::mat3_cast(rotation_) * forward; // Use mat3_cast
  }

  glm::vec3 getUp() const {
      glm::vec3 up(0.0f, 0.0f, 1.0f);
      return glm::mat3_cast(rotation_) * up; // Use mat3_cast
  }

  glm::vec3 getRight() const {
      glm::vec3 right(1.0f, 0.0f, 0.0f);
      return glm::mat3_cast(rotation_) * right; // Use mat3_cast
  }
  void setForward(const glm::vec3& forward) {
      glm::vec3 currentForward = getForward();
      glm::quat rotation = glm::quatLookAt(currentForward, forward);
      setRotation(rotation);
  }
  void setUp(const glm::vec3& up) {
      glm::vec3 currentUp = getUp();
      glm::quat rotation = glm::quatLookAt(currentUp, up);
      setRotation(rotation);
  }
  void setRight(const glm::vec3& right) {
      glm::vec3 currentRight = getRight();
      glm::quat rotation = glm::quatLookAt(currentRight, right);
      setRotation(rotation);
  }

  // Rotation
  glm::quat getRotation() const { 
    return rotation_; 
  }
  void setRotation(const glm::quat& rotation) { 
    rotation_ = rotation; 
  }

  void setRotationFromYawPitch(float yaw, float pitch) {
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    rotation_ = qYaw * qPitch;
  }

  void setYaw(float yaw) {
    glm::quat qPitch = glm::angleAxis(glm::pitch(rotation_), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    rotation_ = qYaw * qPitch;
  }

  void setPitch(float pitch) {
    glm::quat qYaw = glm::angleAxis(glm::yaw(rotation_), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    rotation_ = qYaw * qPitch;
  }

  float getYaw() const {
    glm::vec3 euler = glm::eulerAngles(rotation_);
    return glm::degrees(euler.z);
  }

  float getPitch() const {
    glm::vec3 euler = glm::eulerAngles(rotation_);
    return glm::degrees(euler.x);
  }

  // Scale
  glm::vec3 getScale() const { 
    return scale_; 
  }
  void setScale(const glm::vec3& scale) { 
    scale_ = scale; 
  }

  void setParentModel(const glm::mat4& parentModel) { 
    parent_ = parentModel; 
  }
  glm::mat4 getParentModel() const { 
    return parent_; 
  }

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

  void log() const {
    Log::console("Transform data: " + name_, 1);
    Log::console(position_, "Position: ", 2); 
    Log::console(rotation_, "Rotation: ", 2); 
    Log::console(scale_, "Scale: ", 2); 
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