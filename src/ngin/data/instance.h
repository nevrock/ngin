#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

#include <fstream> // Add this include directive
#include <sstream> // Add this include directive

#include <glad/glad.h>
#include <glm/glm.hpp>

struct InstanceData {
    glm::mat4 model;
    glm::vec2 uv_offset;
    glm::vec3 color;
};

#endif