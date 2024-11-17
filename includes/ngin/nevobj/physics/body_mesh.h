#ifndef BODY_MESH_H
#define BODY_MESH_H

#include <glm/vec3.hpp>
#include "body_collider.h"
#include <snorri/mesh.h>  // Include path for Mesh might need to be updated based on your project structure

class BodyMesh : public BodyCollider {
public:
    Mesh* mesh;  // Pointer to mesh data

    // Constructor for the mesh body
    BodyMesh(Mesh* mesh, const glm::vec3& position, const glm::vec3& size, const glm::vec3& velocity, float mass);

    // Virtual methods for collision detection and resolution
    bool intersects(const BodyCollider& other) const override;
    void resolveCollision(BodyCollider& other) override;

    bool intersectsRay(const Ray& ray, float& tmin, float& tmax) const override;

private:
    std::vector<glm::vec3> meshAxes;  // Store the precomputed axes for SAT

    // Private methods for collision calculations
    std::vector<glm::vec3> generateAxesForSAT(const Mesh& mesh1, const Mesh& mesh2) const;
    std::vector<glm::vec3> generateAxesForSAT() const;
    void calculateMeshAxes(const Mesh& mesh1);
    std::vector<glm::vec3> generateAxesForSATWithBox(const Mesh& mesh, const BodyCollider& box) const;
    bool overlapOnAxis(const glm::vec3& axis, const Mesh& mesh1, const BodyCollider& other) const;
    std::vector<glm::vec3> generateBoxCorners(const glm::vec3& position, const glm::vec3& size) const;
    glm::vec3 calculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) const;
    float project(const glm::vec3& point, const glm::vec3& axis) const;
};

#endif // BODY_MESH_H
