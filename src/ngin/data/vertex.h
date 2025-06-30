#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

#include <glm/glm.hpp>

#include <vector>
#include <string>

#define MAX_BONE_INFLUENCE 4
#define MAX_TEXTURES 4

struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
	int bone_ids[MAX_BONE_INFLUENCE];
	float bone_weights[MAX_BONE_INFLUENCE];
};

#endif // VERTEX_DATA_H