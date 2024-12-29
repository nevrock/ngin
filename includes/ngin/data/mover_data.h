#ifndef MOVER_DATA_H
#define MOVER_DATA_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ngin/data/i_data.h>
#include <ngin/collections/nevf.h> // Include the header file where Nevf is defined

class MoverData : public IData {
public:

  MoverData() : positionDelta_(glm::vec3(0.0)), rotationDelta_(glm::quat()), scaleDelta_(glm::vec3(0.0)) {} 
  MoverData(glm::vec3 pos, glm::quat rot, glm::vec3 scale) : positionDelta_(pos), rotationDelta_(rot), scaleDelta_(scale) {

  }
  MoverData(const Nevf& data) {
    if (data.contains("position")) {
      positionDelta_ = data.getVec("position", glm::vec3(0.0));
    }
    if (data.contains("rotation")) {
      glm::vec4 rotationVec = data.getVec4("rotation", glm::vec4(0.0f));
      rotationDelta_ = glm::quat(rotationVec.w, rotationVec.x, rotationVec.y, rotationVec.z);
    }
    if (data.contains("scale")) {
      scaleDelta_ = data.getVec("scale", glm::vec3(1.0));
    }
  }

  glm::vec3 getPositionDelta() const { return positionDelta_; }
  void setPositionDelta(glm::vec3 pos) {
    positionDelta_ = pos;
  }
  glm::quat getRotationDelta() const { return rotationDelta_; }
  void setRotationDelta(glm::quat rot) {
    rotationDelta_ = rot;
  }
  glm::vec3 getScaleDelta() const { return scaleDelta_; }
  void setScaleDelta(glm::vec3 scale) {
    scaleDelta_ = scale;
  }

  void execute() override {
    // cook this data so it is stable for remaining passes
  }
  std::string getName() override { return name_; } 


protected:
  std::string name_;

  glm::vec3 positionDelta_;
  glm::quat rotationDelta_;
  glm::vec3 scaleDelta_;
};

#endif