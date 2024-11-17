#ifndef BODYBOX_H
#define BODYBOX_H

#include <glm/vec3.hpp>
#include "body_collider.h"

struct AABB {
    glm::vec3 minCorner;
    glm::vec3 maxCorner;
};

class BodyBox : public BodyCollider {
public:
    BodyBox(const glm::vec3& position, const glm::vec3& size, const glm::vec3& velocity, float mass);
    BodyBox();

    // Check if this box intersects with another box
    bool intersects(const BodyCollider& other) const override;

    // Resolve collision modifying the velocity of this box based on another box
    void resolveCollision(BodyCollider& other) override;

    // Methods to set position and velocity
    void setPosition(const glm::vec3& newPosition) override;
    void setVelocity(const glm::vec3& newVelocity) override;

    bool intersectsRay(const Ray& ray, float& tmin, float& tmax) const override;

    AABB getBox() const;
};

#endif // BODYBOX_H
