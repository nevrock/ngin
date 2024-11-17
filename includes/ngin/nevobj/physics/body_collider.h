#ifndef BODY_COLLIDER_H
#define BODY_COLLIDER_H

#include <glm/vec3.hpp>
#include <iostream>
#include <snorri/ray.h>

class BodyCollider {
public:
    glm::vec3 position;  // Center position of the box
    glm::vec3 size;      // Dimensions of the box (extends in each direction from the center)
    glm::vec3 velocity;  // Velocity vector of the box
    float mass;          // Mass of the box

    BodyCollider(const glm::vec3& position, const glm::vec3& size, const glm::vec3& velocity, float mass)
        : position(position), size(size), velocity(velocity), mass(mass) 
    {

    }
    BodyCollider() {

    }

    virtual bool intersects(const BodyCollider& other) const = 0;

    virtual bool intersectsRay(const Ray& ray, float& tMin, float& tMax) const = 0;

    virtual void resolveCollision(BodyCollider& other) = 0;

    virtual void setPosition(const glm::vec3& newPosition) {
        position = newPosition;
    }
    virtual void setVelocity(const glm::vec3& newVelocity) {
        velocity = newVelocity;
    }

    glm::vec3 getVelocity() {
        return velocity;
    }

    const std::string& getType() const {  // Added 'const' at the start and 'const' at the end
        return type;
    }

protected:
    std::string type;
};

#endif // BODYBOX_H
