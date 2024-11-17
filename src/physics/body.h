#ifndef BODY_H
#define BODY_H

#include <memory>
#include <glm/glm.hpp>
#include <snorri/object.h>
#include "body_collider.h"
#include <snorri/dict.h>

class Body : public Component, public std::enable_shared_from_this<Body> {
public:
    Body(Object* parent);
    ~Body() override;

    void launch() override;
    void loadFromDict(const Dict& d) override;
    void updatePhysics(float dt) override;

    glm::vec3 getVelocity(float dt);
    glm::vec3 getPosition();
    BodyCollider& getCollider();  // Change this method signature

    std::string& getType() {
        return type;
    }

    bool isKinematic;

private:
    float mass;
    glm::vec3 size;
    std::unique_ptr<BodyCollider> bodyCollider;  // Changed to unique_ptr

    std::string type;

    glm::vec3 desiredVelocity;
    glm::vec3 currentVelocity;

    bool isGravity;
};

#endif // BODY_H
