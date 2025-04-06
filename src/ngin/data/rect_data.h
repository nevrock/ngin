#ifndef RECT_DATA_H
#define RECT_DATA_H

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ngin/data/transform_data.h>
#include <ngin/ngin.h>

class RectData : public TransformData {
public:

  RectData() : TransformData(), position_(0.0f), 
              rotation_(0.0f), 
              size_(1.0f),
              anchor_(0.0f),
              pivot_(0.0f) {} 
  RectData(const Lex& data) : TransformData(data), position_(0.0f), 
              rotation_(0.0f), 
              size_(1.0),
              anchor_(0.0f),
              pivot_(0.0f) {
    if (data.contains("position")) {
      position_ = data.getVec2("position", glm::vec2(0.0));
    }
    if (data.contains("rotation")) {
      rotation_ = data.getC<float>("rotation", 0.0);
    }
    if (data.contains("size")) {
      size_ = data.getVec2("size", glm::vec2(1.0));
    }
    if (data.contains("anchor")) {
      anchor_ = data.getVec2("anchor", glm::vec2(0.0f, 0.0f));
    }
    if (data.contains("pivot")) {
      pivot_ = data.getVec2("pivot", glm::vec2(0.0f, 0.0f));
    }

    isRaycast_ = data.getC<bool>("isRaycast", false);

    screenWidth_ = Ngin::envget<int>("screen.width");
    screenHeight_ = Ngin::envget<int>("screen.height");

    parent_ = getDefaultRectMatrix();
  }

  void execute() override {
    // cook this data so it is stable for remaining passes
  }

  // Position
  glm::vec2 getPosition() const { 
    return position_; 
  }
  void setPosition(const glm::vec2& position) { 
    position_ = position; 
  }
  glm::vec2 getWorldPosition() const {
    return glm::vec2(getRectMatrix()[0][0], getRectMatrix()[0][1]);
  }

  // Rotation
  float getRotation() const { 
    return rotation_; 
  }
  void setRotation(const float rotation) { 
    rotation_ = rotation; 
  }


  glm::vec2 getAnchor() const { 
    return anchor_; 
  }

  glm::mat4 getWorldModelMatrix() const override {
    return getRectMatrix();
  }
  glm::mat4 getModelMatrix() const override {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 rect = getRectMatrix();

    float ndcX = (2.0f * rect[0][0]) / screenWidth_ - 1.0f;
    float ndcY = (2.0f * rect[0][1]) / screenHeight_ - 1.0f; // Adjusted to flip the Y-axis

    float ndcWidth = (2.0f * rect[1][0]) / screenWidth_;
    float ndcHeight = (2.0f * rect[1][1]) / screenHeight_;
    
    model = glm::translate(model, glm::vec3(ndcX, ndcY, 0.0f));

    model = glm::translate(model, glm::vec3((1.0) * (ndcWidth/2), (1.0) * (ndcHeight/2), 0.0f));
    
    model = glm::scale(model, glm::vec3(ndcWidth/2, ndcHeight/2, 1.0f));

    // Incorporate the parent_ model matrix
    return model;
  }
  glm::mat4 getRectMatrix() const {
    glm::mat4 rectMatrix = glm::mat4(1.0f);

    glm::vec2 parentPosition = glm::vec2(parent_[0][0], parent_[0][1]);
    glm::vec2 parentSize = glm::vec2(parent_[1][0], parent_[1][1]);
    glm::vec2 parentAnchor = glm::vec2(parent_[2][0], parent_[2][1]);
    glm::vec2 parentPivot = glm::vec2(parent_[3][0], parent_[3][1]);

    glm::vec2 parentOrigin = parentPosition + (parentAnchor * parentSize);

    // Store position in the first row
    rectMatrix[0][0] = position_.x + parentOrigin.x - (pivot_.x * size_.x);
    rectMatrix[0][1] = position_.y + parentOrigin.y - (pivot_.y * size_.y);

    // Store size in the second row
    rectMatrix[1][0] = size_.x;
    rectMatrix[1][1] = size_.y;

    // Store anchor in the third row
    rectMatrix[2][0] = anchor_.x;
    rectMatrix[2][1] = anchor_.y;

    // Store pivot in the fourth row
    rectMatrix[3][0] = pivot_.x;
    rectMatrix[3][1] = pivot_.y;

    return rectMatrix;
    
  }
  glm::mat4 getDefaultRectMatrix() const {
    glm::mat4 rectMatrix = glm::mat4(1.0f);

    // Store position in the first row
    rectMatrix[0][0] = 0.0f;
    rectMatrix[0][1] = 0.0f;

    // Store size in the second row
    rectMatrix[1][0] = 1.0f;
    rectMatrix[1][1] = 1.0f;

    // Store anchor in the third row
    rectMatrix[2][0] = 0.0f;
    rectMatrix[2][1] = 0.0f;

    // Store pivot in the fourth row
    rectMatrix[3][0] = 0.0f;
    rectMatrix[3][1] = 0.0f;

    return rectMatrix;
  }

  void log() const override {
    Log::console("Rect data: " + name_, 1);
    Log::console(position_, "Position: ", 2); 
    Log::console("Rotation: " + std::to_string(rotation_), 2); 
  }
  
  bool raycast(const glm::vec2& ray) const {
    if (!isRaycast_) {
      return false;
    }
    // Ray is raycast screen position - return if in bounds
    glm::mat4 rect = getRectMatrix();
    glm::vec2 position = glm::vec2(rect[0][0], rect[0][1]);
    glm::vec2 size = glm::vec2(rect[1][0], rect[1][1]);
    // Implement raycasting logic here
    if (ray.x >= position.x && ray.x <= position.x + size.x &&
        ray.y >= position.y && ray.y <= position.y + size.y) {
      return true;
    }
    return false;
  }

private:
  glm::vec2 position_;  
  glm::vec2 size_;
  glm::vec2 anchor_;
  glm::vec2 pivot_;

  float rotation_;

  int screenWidth_, screenHeight_;

  bool isRaycast_ = false;
};

#endif