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
        glm::vec3 penetration;
        penetration.x = (size.x + other.size.x) * 0.5f - abs(position.x - other.position.x);
        penetration.y = (size.y + other.size.y) * 0.5f - abs(position.y - other.position.y);
        penetration.z = (size.z + other.size.z) * 0.5f - abs(position.z - other.position.z);

        bool intersects = penetration.x > 0 && penetration.y > 0 && penetration.z > 0;

        if (intersects) {
            std::cout << "Intersection detected! Direction: ";

            if (penetration.x < penetration.y && penetration.x < penetration.z) {
                std::cout << "X-axis";
            } else if (penetration.y < penetration.z) {
                std::cout << "Y-axis";
            } else {
                std::cout << "Z-axis";
            }

            std::cout << std::endl;
            std::cout << "This box position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
            std::cout << "Other box position: (" << other.position.x << ", " << other.position.y << ", " << other.position.z << ")" << std::endl; 
        }

        return intersects;
    } else if (other.getType() == "mesh") {
        return other.intersects(*this); 
    } 
    return false; 
}

void BodyBox::resolveCollision(BodyCollider& other) {
    if (other.getType() == "box") {
        glm::vec3 penetration;
        penetration.x = (size.x + other.size.x) * 0.5f - abs(position.x - other.position.x);
        penetration.y = (size.y + other.size.y) * 0.5f - abs(position.y - other.position.y);
        penetration.z = (size.z + other.size.z) * 0.5f - abs(position.z - other.position.z);

        // Find the axis of minimum penetration
        if (penetration.x < penetration.y && penetration.x < penetration.z) {
            // Collision on the x-axis
            if (position.x < other.position.x) {
                position.x -= penetration.x; 
                velocity.x = std::min(velocity.x, 0.0f); // Adjust velocity based on direction
            } else {
                position.x += penetration.x;
                velocity.x = std::max(velocity.x, 0.0f); 
            }
        } else if (penetration.y < penetration.z) {
            // Collision on the y-axis
            if (position.y < other.position.y) {
                position.y -= penetration.y;
                velocity.y = std::min(velocity.y, 0.0f); 
            } else {
                position.y += penetration.y;
                velocity.y = std::max(velocity.y, 0.0f); 
            }
        } else {
            // Collision on the z-axis
            if (position.z < other.position.z) {
                position.z -= penetration.z;
                velocity.z = std::min(velocity.z, 0.0f); 
            } else {
                position.z += penetration.z;
                velocity.z = std::max(velocity.z, 0.0f); 
            }
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