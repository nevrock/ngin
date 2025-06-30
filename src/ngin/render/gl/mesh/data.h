#ifndef GL_MESH_DATA_H
#define GL_MESH_DATA_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ngin/data/mesh.h>
#include <ngin/data/instance.h>

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

class GlMeshData {
public:
    GlMeshData(MeshData& mesh)
        : mesh_(mesh), vao_(0), vbo_(0), ebo_(0), instance_vbo_(0), num_indices_(0) {}

    ~GlMeshData() {
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
        if (instance_vbo_ != 0) glDeleteBuffers(1, &instance_vbo_);
    }

    void refresh() {
        std::vector<VertexData>& vertices = mesh_.vertices;
        std::vector<FaceData>& faces = mesh_.faces;

        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

        std::vector<unsigned int> indices;
        indices.reserve(faces.size() * 3 * 2); // Reserve enough space for average cases (triangles or quads triangulated)
        for (const auto& face : faces) {
            // Each face's triangles are added to the global index list.
            for (const auto& triangle : face.triangles) {
                indices.push_back(triangle[0]);
                indices.push_back(triangle[1]);
                indices.push_back(triangle[2]);
            }
        }
        num_indices_ = indices.size();

        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Layout Location 0: Vertex Positions (vec3)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
        // Layout Location 1: Vertex Normals (vec3)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
        // Layout Location 2: Vertex UVs (vec2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));
        // Layout Location 3: Vertex Colors (vec3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));
        // Layout Location 4: Bone IDs (ivec4) - Integer attribute pointer
        glEnableVertexAttribArray(4);
        glVertexAttribIPointer(4, MAX_BONE_INFLUENCE, GL_INT, sizeof(VertexData), (void*)offsetof(VertexData, bone_ids));
        // Layout Location 5: Bone Weights (vec4)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, bone_weights));

        glGenBuffers(1, &instance_vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, model));
        glVertexAttribDivisor(6, 1);
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, model) + sizeof(glm::vec4)));
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, model) + 2 * sizeof(glm::vec4)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, model) + 3 * sizeof(glm::vec4)));
        glVertexAttribDivisor(9, 1);

        // Layout Location 10: UV Offset (vec2)
        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, uv_offset));
        glVertexAttribDivisor(10, 1);

        // Layout Location 11: Color (vec3)
        glEnableVertexAttribArray(11);
        glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, color));
        glVertexAttribDivisor(11, 1);

        // Unbind the VAO to prevent accidental modification.
        glBindVertexArray(0);
    }
    void render() {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void render_instances(const std::vector<InstanceData>& instances) {
        if (instances.empty()) return; // Nothing to render

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);

        glDrawElementsInstanced(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0, instances.size());

        glBindVertexArray(0);
    }

private:
    unsigned int vao_;         // Vertex Array Object ID
    unsigned int vbo_;         // Vertex Buffer Object ID (for per-vertex data)
    unsigned int ebo_;         // Element Buffer Object ID (for indices)
    unsigned int instance_vbo_; // Vertex Buffer Object ID (for per-instance data)

    size_t num_indices_; // Cached total number of indices to draw
    MeshData& mesh_; // Reference to the MeshData
};


#endif // GL_MESH_DATA_H
