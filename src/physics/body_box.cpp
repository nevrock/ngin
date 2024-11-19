#include "body_box.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>

BodyBox::BodyBox(const glm::vec3& position, const glm::vec3& size, const glm::vec3& velocity, float mass)
    : BodyCollider(position, size, velocity, mass) {
    type = "box";
}

BodyBox::BodyBox() { type = "box"; }

bool BodyBox::intersects(const BodyCollider& other) const {
    if (other.getType() == "box") {
        return (abs(position.x - other.position.x) <= (size.x + other.size.x) ) ||
           (abs(position.y - other.position.y) <= (size.y + other.size.y) ) ||
           (abs(position.z - other.position.z) <= (size.z + other.size.z));
    } else if (other.getType() == "mesh") {
        return other.intersects(*this);
    }
}

void BodyBox::resolveCollision(BodyCollider& other) {
    if (other.getType() == "box") {
        if (((position.y - other.position.y) <= (size.y + other.size.y))) {
            //below
            if (velocity.y < 0.0)
                velocity.y = 0.0;
        } else if ((abs(position.y - other.position.y) <= (size.y + other.size.y))) {
            // above
            if (velocity.y > 0.1)
                velocity.y = 0.0;
        }
        if (((position.x - other.position.x) <= (size.x + other.size.x))) {
            //left
            if (velocity.x < 0.0)
                velocity.x = 0.0;
        } else if ((abs(position.x - other.position.x) <= (size.x + other.size.x))) {
            // right
            if (velocity.x > 0.1)
                velocity.x = 0.0;
        }
        if (((position.z - other.position.z) <= (size.z + other.size.z) )) {
            // front
            if (velocity.z > 0.1)
                velocity.z = 0.0;
        } else if ((abs(position.z - other.position.z) <= (size.z + other.size.z))) {
            // back
            if (velocity.z < 0.0)
                velocity.z = 0.0;
        }
    } else if (other.getType() == "mesh") {
        other.resolveCollision(*this);
    }
}

void BodyBox::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

void BodyBox::setVelocity(const glm::vec3& newVelocity) {
    velocity = newVelocity;
}

AABB BodyBox::getBox() const {
    AABB box;
    box.minCorner = position - (size * 0.5f);  // Use multiplication instead of division
    box.maxCorner = position + (size * 0.5f);  // Consistently use multiplication
    return box;
}

bool BodyBox::intersectsRay(const Ray& ray, float& tmin, float& tmax) const {
    float t1, t2;
    tmin = std::numeric_limits<float>::lowest();
    tmax = std::numeric_limits<float>::max();

    AABB box = getBox();

    for (int i = 0; i < 3; ++i) {
        float dirComponent = (i == 0 ? ray.direction.x : (i == 1 ? ray.direction.y : ray.direction.z));
        float originComponent = (i == 0 ? ray.origin.x : (i == 1 ? ray.origin.y : ray.origin.z));
        
        // Avoid division by zero by checking if the direction component is almost zero
        if (std::abs(dirComponent) < std::numeric_limits<float>::epsilon()) {
            // Check if origin is outside the bounding slab
            if (originComponent < (i == 0 ? box.minCorner.x : (i == 1 ? box.minCorner.y : box.minCorner.z)) || 
                originComponent > (i == 0 ? box.maxCorner.x : (i == 1 ? box.maxCorner.y : box.maxCorner.z))) {
                return false;
            }
        } else {
            float invD = 1.0f / dirComponent;
            t1 = ((i == 0 ? box.minCorner.x : (i == 1 ? box.minCorner.y : box.minCorner.z)) - originComponent) * invD;
            t2 = ((i == 0 ? box.maxCorner.x : (i == 1 ? box.maxCorner.y : box.maxCorner.z)) - originComponent) * invD;

            if (invD < 0.0f) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmax < tmin) return false; // No intersection
        }
    }

    return true; // Intersection occurs
}