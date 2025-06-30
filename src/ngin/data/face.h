#ifndef FACE_DATA_H
#define FACE_DATA_H

#include <glm/glm.hpp>
#include <vector>
#include <array> // For std::array
#include <string> // Keep if you plan to add string members later

struct FaceData {
    std::vector<int> indices;
    // Use std::array for fixed-size arrays, or glm::ivec3 for a more GLM-centric approach
    std::vector<std::array<int, 3>> triangles; 
    // OR: std::vector<glm::ivec3> triangles; // If you prefer GLM's integer vectors
    glm::vec3 origin;

    // Default constructor to ensure members are initialized
    FaceData() : origin(0.0f, 0.0f, 0.0f) {}

    void triangulate() {
        triangles.clear(); // Clear existing triangles before re-triangulating

        if (indices.size() < 3) { // A face needs at least 3 vertices to form a triangle
            // Handle error or return if not enough indices for a triangle
            // For now, we'll just return
            return; 
        }

        if (indices.size() == 3) { // Already a triangle
            triangles.push_back({indices[0], indices[1], indices[2]});
        } else { // Polygon with 4 or more vertices (assumed convex for simple fan triangulation)
            for (size_t i = 1; i < indices.size() - 1; ++i) {
                triangles.push_back({indices[0], indices[i], indices[i+1]});
            }
        }
    }
};

#endif // FACE_DATA_H