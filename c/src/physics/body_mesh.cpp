#define NOMINMAX  // Prevents the Windows headers from defining min/max macros

#include "body_mesh.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <nev/log.h>
#include <nev/mathutils.h>

BodyMesh::BodyMesh(Mesh* mesh, const glm::vec3& position, const glm::vec3& size, const glm::vec3& velocity, float mass)
    : BodyCollider(position, size, velocity, mass), mesh(mesh) {
    type = "mesh";
    calculateMeshAxes(*mesh);
}

bool BodyMesh::intersectsRay(const Ray& ray, float& tMin, float& tMax) const {
    tMin = 0; // Start of the ray
    tMax = 1e6f;  // A large number, but not as large as the max float.

    for (const auto& normal : meshAxes) {
        glm::vec3 p0 = ray.origin - position; // Translate ray origin to object space
        float denominator = glm::dot(ray.direction, normal);

        if (fabs(denominator) > 1e-6) { // Ensure normal is not orthogonal to ray direction
            float t = -glm::dot(p0, normal) / denominator;
            if (denominator < 0) {
                tMin = MathUtils::getMax(tMin, t);
            } else {
                tMax = MathUtils::getMin(tMax, t);
            }

            // If at any point the min exceeds the max, no intersection is possible
            if (tMin > tMax) {
                return false;
            }
        } else {
            // Ray is parallel to triangle plane
            if (glm::dot(p0, normal) > 0) {
                return false; // Ray is not intersecting the plane
            }
        }
    }

    return true; // Bounding values are within valid range, intersection is true
}

bool BodyMesh::intersects(const BodyCollider& other) const {
    std::vector<glm::vec3> axes;

    if (other.getType() == "mesh") {
        const BodyMesh& otherMesh = static_cast<const BodyMesh&>(other);
        axes = generateAxesForSAT(*mesh, *otherMesh.mesh);
    } else if (other.getType() == "box") {
        // Log::console("body mesh intersect check with box!");
        // Generate axes for SAT between mesh and box
        axes = generateAxesForSATWithBox(*mesh, other);
    }

    // Check overlap along all axes
    for (const auto& axis : axes) {
        if (!overlapOnAxis(axis, *mesh, other)) {
            return false; // Found a separating axis
        }
    }
    
    return true; // No separating axis found, they intersect
}
std::vector<glm::vec3> BodyMesh::generateAxesForSATWithBox(const Mesh& mesh, const BodyCollider& box) const {
    std::vector<glm::vec3> axes = generateAxesForSAT(); // Get mesh axes

    // Add box axes (assuming box is axis-aligned)
    axes.push_back(glm::vec3(1, 0, 0)); // X-axis
    axes.push_back(glm::vec3(0, 1, 0)); // Y-axis
    axes.push_back(glm::vec3(0, 0, 1)); // Z-axis

    return axes;
}

std::vector<glm::vec3> BodyMesh::generateAxesForSAT() const {
    return meshAxes;
}
void BodyMesh::calculateMeshAxes(const Mesh& mesh1) {
    meshAxes.reserve(mesh1.indices.size());
    for (size_t i = 0; i < mesh1.indices.size(); i += 3) {
        glm::vec3 normal = calculateNormal(
            mesh1.vertices[mesh1.indices[i]].position,
            mesh1.vertices[mesh1.indices[i+1]].position,
            mesh1.vertices[mesh1.indices[i+2]].position);
        meshAxes.push_back(normal);
    }
}
std::vector<glm::vec3> BodyMesh::generateAxesForSAT(const Mesh& mesh1, const Mesh& mesh2) const {
    std::vector<glm::vec3> axes;

    // Assuming all triangle normals are unique axes
    for (size_t i = 0; i < mesh1.indices.size(); i += 3) {
        glm::vec3 normal = calculateNormal(mesh1.vertices[mesh1.indices[i]].position,
                                           mesh1.vertices[mesh1.indices[i+1]].position,
                                           mesh1.vertices[mesh1.indices[i+2]].position);
        axes.push_back(normal);
    }
    for (size_t i = 0; i < mesh2.indices.size(); i += 3) {
        glm::vec3 normal = calculateNormal(mesh2.vertices[mesh2.indices[i]].position,
                                           mesh2.vertices[mesh2.indices[i+1]].position,
                                           mesh2.vertices[mesh2.indices[i+2]].position);
        axes.push_back(normal);
    }

    return axes;
}

bool BodyMesh::overlapOnAxis(const glm::vec3& axis, const Mesh& mesh1, const BodyCollider& other) const {
    float minA = std::numeric_limits<float>::max(), maxA = std::numeric_limits<float>::lowest();
    float minB = std::numeric_limits<float>::max(), maxB = std::numeric_limits<float>::lowest();

    // Project all vertices of mesh1
    for (const auto& vertex : mesh1.vertices) {
        float projection = project(vertex.position + position, axis);  // Adjust vertex position by mesh's world position
        minA = std::min(minA, projection);
        maxA = std::max(maxA, projection);
    }

    // Project all vertices of other collider, check if it's a mesh or a box
    if (other.getType() == "mesh") {
        const Mesh& mesh2 = *(static_cast<const BodyMesh&>(other).mesh);
        for (const auto& vertex : mesh2.vertices) {
            float projection = project(vertex.position + other.position, axis);  // Adjust vertex position by other mesh's world position
            minB = std::min(minB, projection);
            maxB = std::max(maxB, projection);
        }
    } else if (other.getType() == "box") {
        // Project the box's corners
        std::vector<glm::vec3> corners = generateBoxCorners(other.position, other.size);
        for (const auto& corner : corners) {
            float projection = project(corner, axis);
            minB = std::min(minB, projection);
            maxB = std::max(maxB, projection);
        }
    }

    // Check if the projections overlap
    return maxA >= minB && maxB >= minA;
}
std::vector<glm::vec3> BodyMesh::generateBoxCorners(const glm::vec3& position, const glm::vec3& size) const {
    std::vector<glm::vec3> corners;
    glm::vec3 halfSize = size * 0.5f;
    corners.push_back(position + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z));
    corners.push_back(position + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z));
    corners.push_back(position + glm::vec3(-halfSize.x, halfSize.y, -halfSize.z));
    corners.push_back(position + glm::vec3(halfSize.x, halfSize.y, -halfSize.z));
    corners.push_back(position + glm::vec3(-halfSize.x, -halfSize.y, halfSize.z));
    corners.push_back(position + glm::vec3(halfSize.x, -halfSize.y, halfSize.z));
    corners.push_back(position + glm::vec3(-halfSize.x, halfSize.y, halfSize.z));
    corners.push_back(position + glm::vec3(halfSize.x, halfSize.y, halfSize.z));
    return corners;
}

glm::vec3 BodyMesh::calculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) const {
    return glm::normalize(glm::cross(v2 - v1, v3 - v1));
}

float BodyMesh::project(const glm::vec3& point, const glm::vec3& axis) const {
    return glm::dot(point, axis) / glm::length(axis);
}

void BodyMesh::resolveCollision(BodyCollider& other) {
    std::vector<glm::vec3> axes;

    if (other.getType() == "box") {
        std::vector<glm::vec3> axes = generateAxesForSATWithBox(*mesh, other);

        for (const auto& axis : axes) {
            if (overlapOnAxis(axis, *mesh, other)) {
                // Calculate the relative velocity along the collision axis
                float velocityComponent = glm::dot(other.velocity, axis);

                if (velocityComponent != 0) {
                    // Stop the box collider's velocity along the collision axis
                    other.velocity -= axis * velocityComponent;
                }
            }
        }
    }
}
