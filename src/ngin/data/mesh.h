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

    void from_data(Atlas& data, Logger* logger = nullptr) {
        Atlas* raw_data = new Atlas();
        raw_data = data.get<Atlas>("data", raw_data);

        // vertices
        Atlas* vertex_data = new Atlas();
        vertex_data = raw_data->get<Atlas>("vertices", vertex_data);
        for (const auto& key : vertex_data->keys()) {
            Atlas* vertex = new Atlas();
            vertex = vertex_data->get<Atlas>(key, vertex);
            
            VertexData v;
            std::vector<float>* position_vec = vertex->get<std::vector<float>>("position", nullptr);
            if (position_vec) {
                v.position = glm::vec3(position_vec->at(0), position_vec->at(1), position_vec->at(2));
            }
            std::vector<float>* normal_vec = vertex->get<std::vector<float>>("normal", nullptr);
            if (normal_vec) {
                v.normal = glm::vec3(normal_vec->at(0), normal_vec->at(1), normal_vec->at(2));
            }
            std::vector<float>* uv_vec = vertex->get<std::vector<float>>("uv", nullptr);
            if (uv_vec) {
                v.uv = glm::vec2(uv_vec->at(0), uv_vec->at(1));
            }
            std::vector<float>* color_vec = vertex->get<std::vector<float>>("color", nullptr);
            if (color_vec) {
                v.color = glm::vec3(color_vec->at(0), color_vec->at(1), color_vec->at(2));
            }
            std::vector<int>* bone_ids_vec = vertex->get<std::vector<int>>("bone_ids", nullptr);
            std::vector<float>* bone_weights_vec = vertex->get<std::vector<float>>("bone_weights", nullptr);
            if (bone_ids_vec && bone_weights_vec) {
                for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                    v.bone_ids[i] = bone_ids_vec->at(i);
                    v.bone_weights[i] = bone_weights_vec->at(i);
                }            
            }

            vertices.push_back(v);
        }

        // faces
        Atlas* face_data = new Atlas();
        face_data = raw_data->get<Atlas>("faces", face_data);
        for (const auto& key : face_data->keys()) {
            std::vector<int>* face = new std::vector<int>();
            face = face_data->get<std::vector<int>>(key, face);
            FaceData f;
            f.indices = *face;
            f.triangulate();

            // calculate origin
            glm::vec3 origin = glm::vec3(0.0f);
            for (int i = 0; i < face->size(); i++) {
                origin += vertices[face->at(i)].position;
            }
            origin /= face->size();
            f.origin = origin;

            faces.push_back(f);

            // if (face) {
            //     if (face->size() > 3) { // Triangulate from Blender export (quads or n-gons)
            //         // Assuming the indices are ordered to form convex polygons
            //         for (size_t i = 1; i < face->size() - 1; ++i) {
            //             FaceData f;
            //             f.indices[0] = face->at(0);
            //             f.indices[1] = face->at(i);
            //             f.indices[2] = face->at(i + 1);
            //             faces.push_back(f);
            //         }
            //     } else if (face->size() == 3) {
            //         FaceData f;
            //         f.indices[0] = face->at(0);
            //         f.indices[1] = face->at(1);
            //         f.indices[2] = face->at(2);
            //         faces.push_back(f);
            //     }
            // }
        }

        if (logger) {
            logger->info(
                "Mesh created with " + std::to_string(vertices.size()) + 
                " vertices, & " + std::to_string(faces.size()) + " faces", 3);
        }
    }
};

#endif // MESH_DATA_H