#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

#include <fstream> // Add this include directive
#include <sstream> // Add this include directive

#include <glad/glad.h>
#include <glm/glm.hpp>

struct InstanceData {
    glm::mat4 modelMatrix;
    glm::vec2 uvOffset;
    glm::vec3 color;
};

#endif