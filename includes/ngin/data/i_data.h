#ifndef I_DATA_H 
#define I_DATA_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class IData {
public:
  virtual std::string getName() = 0; 
  virtual void execute() = 0;
};

#endif // I_DATA_H
