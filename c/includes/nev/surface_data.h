#ifndef SURFACE_DATA_H
#define SURFACE_DATA_H

#include <glad/glad.h>
#include <glm/glm.hpp>

struct SurfaceData {
    glm::vec4 position;
    glm::vec4 rotation;
    glm::vec4 scale;
    glm::vec4 diffuse;
    int shapeType;
    int blendType;
    float blendStrength;
    int outline;
};

#endif
