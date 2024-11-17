#include "body.h"
#include "body_box.h"
#include "body_mesh.h"
#include "physics_manager.h"
#include <nev/log.h>
#include <glm/glm.hpp>
#include <iostream>

#include <nev/resources.h>

Body::Body(Object* parent) 
    : Component(parent), mass(0.0), size(glm::vec3(1,1,1)), desiredVelocity(glm::vec3(0,0,0)),
        currentVelocity(glm::vec3(0,0,0)), isGravity(false), isKinematic(false) {
    Log::console("new body!");
}

Body::~Body() {
    //PhysicsManager::unregisterBody(shared_from_this());
}

void Body::loadFromDict(const Dict& d) {
    PhysicsManager::registerBody(shared_from_this());

    type = d.getC<std::string>("type", "box");
    mass = d.getC<float>("mass", 1.0);
    isGravity = d.getC<bool>("is_gravity", false);
    isKinematic = d.getC<bool>("is_kinematic", false);
    
    // Initialize BodyBox with these parameters
    Point& p = getPoint();
    glm::vec3 pos = p.getPosition();

    size = d.getVec("size", glm::vec3(1.0));

    if (type == "box") {
        bodyCollider = std::make_unique<BodyBox>(pos, size, glm::vec3(0), mass);
    } else if (type == "mesh") {

        std::string fileName = d.getC<std::string>("file", "plane");
        ModelData* data = Resources::getModel(fileName);
        bodyCollider = std::make_unique<BodyMesh>(data->getMesh(d.getC<int>("file_index", 0)), pos, size, glm::vec3(0), mass);
       // Log::console("body mesh initiated!");
    }
}

void Body::launch() {
}

BodyCollider& Body::getCollider() {
    return *bodyCollider;
}

void Body::updatePhysics(float dt) {
    if (bodyCollider == nullptr) return;

    if (isKinematic)
        return;

    //Log::console("physics update!");

    Point& p = getPoint();
    glm::vec3 pos = p.getPosition();

    glm::vec3 vel = currentVelocity;
    vel = glm::mix(vel, desiredVelocity + glm::vec3(0,-9.8,0), dt);

    // glm::vec3 vel = getVelocity(dt);

    bodyCollider->setPosition(pos);
    bodyCollider->setVelocity(vel);

    //Log::console(": delta time: " + std::to_string(dt));
    //Log::console(": postion start - " + std::to_string(pos.x) + ", " 
    //    + std::to_string(pos.y) + ", "
    //    + std::to_string(pos.z));
    //Log::console(": velocity started - " + std::to_string(vel.x) + ", " 
    //    + std::to_string(vel.y) + ", "
    //    + std::to_string(vel.z));

    std::vector<std::shared_ptr<Body>> bodies = PhysicsManager::getClosest(pos.x, pos.y, pos.z);
    //Log::console("Number of bodies retrieved for collision checks: " + std::to_string(bodies.size()));
    for (const auto& body : bodies) {
        if (body == shared_from_this())
            continue;

        if (!body->isKinematic)
            continue;

        BodyCollider& otherBox = body->getCollider();

        //std::cout << "Other body position is: " << otherBox.position.x << ", " <<
        //    otherBox.position.y << ", " << otherBox.position.z
        //    << std::endl;
        if (bodyCollider->intersects(otherBox)) {
            //Log::console("body collision registered!");
            bodyCollider->resolveCollision(otherBox);
        }
    }
    currentVelocity = bodyCollider->getVelocity();
    pos += bodyCollider->getVelocity() * dt; // Ensure deltaTime is defined or accessible
    p.setPosition(pos); // Make sure setPosition actually modifies the Body's stored position, or that it's unnecessary based on your architecture

    //Log::console(": postion end - " + std::to_string(pos.x) + ", " 
    //    + std::to_string(pos.y) + ", "
    //    + std::to_string(pos.z));

    //Log::console(": velocity end - " + std::to_string(bodyCollider.velocity.x) + ", " 
    //    + std::to_string(bodyCollider.velocity.y) + ", "
    //    + std::to_string(bodyCollider.velocity.z));
}

glm::vec3 Body::getVelocity(float dt) {
    glm::vec3 vel = currentVelocity;
    vel = glm::mix(vel, desiredVelocity + glm::vec3(0,-9.8,0), dt);
    return vel;
}

glm::vec3 Body::getPosition() {
    return getPoint().getPosition();
}

bool body_registered = []() {
    try {
        Log::console("body registering now");
        Object::registerComponent("body", [](Object* parent) {
            return std::make_shared<Body>(parent);
        });
    } catch (const std::exception& e) {
        Log::console(std::string("Failed to register body: ") + e.what());
        return false;
    }
    return true;
}();
