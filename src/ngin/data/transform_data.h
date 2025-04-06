#ifndef TRANSFORM_DATA_H
#define TRANSFORM_DATA_H

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <ngin/log.h>
#include <ngin/data/i_data.h>

class TransformData : public IData {
public:

  TransformData() : parent_(1.0f) {} 
  TransformData(const Lex& data) : parent_(1.0f) {
    parent_ = glm::mat4(1.0f);
  }
  std::string getName() override { return name_; } 

  void setParentModel(const glm::mat4& parentModel) { 
    parent_ = parentModel; 
  }
  glm::mat4 getParentModel() const { 
    return parent_; 
  }
  virtual glm::mat4 getWorldModelMatrix() const {
    return parent_ * getModelMatrix(); 
  }

  virtual glm::mat4 getModelMatrix() const {
    return glm::mat4(1.0f); 
  }
  virtual void log() const {}

protected:
  std::string name_;
  // this gets set when the graph is cooked from the parent node
  glm::mat4 parent_;
};

#endif