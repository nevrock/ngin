#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
    int bones[MAX_BONES];
    float weights[MAX_BONES];
};

#endif // VERTEX_H
