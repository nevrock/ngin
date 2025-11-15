#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

#include <ngin/data/face.h>
#include <ngin/data/vertex.h>

struct MeshData {
    std::vector<VertexData> vertices;
    std::vector<FaceData> faces;
    std::vector<std::string> vertex_groups;
    bool smooth_shading;

    std::vector<VertexData>& get_vertices() {
        return vertices;
    }
    std::vector<FaceData>& get_faces() {
        return faces;
    }
};

#endif // MESH_DATA_H