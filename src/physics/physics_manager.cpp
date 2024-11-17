#include <snorri/log.h>
#include "body.h" // Include here for complete type information
#include "physics_manager.h"

#include <kdt/kdtree.h>  // Include k-d tree functionality
#include <glm/glm.hpp>   // For mathematical operations
#include <algorithm>     // For std::transform
#include <iterator>      // For std::back_inserter

std::vector<std::shared_ptr<Body>> PhysicsManager::activeBodies;

void PhysicsManager::registerBody(const std::shared_ptr<Body>& body) {
    activeBodies.push_back(body);
    Log::console("Body added. Total bodies: " + std::to_string(activeBodies.size()));
}
void PhysicsManager::unregisterBody(const std::shared_ptr<Body>& body) {
    activeBodies.erase(std::remove(activeBodies.begin(), activeBodies.end(), body), activeBodies.end());
    Log::console("Body removed. Total bodies: " + std::to_string(activeBodies.size()));
}

std::vector<std::shared_ptr<Body>> PhysicsManager::getClosest(double x, double y, double z, std::shared_ptr<Body> excludeBody) {
    std::vector<BodyPoint> points;
    std::vector<std::shared_ptr<Body>> resultBodies;

    // Transform active bodies to points and exclude the specified body
    for (auto& body : activeBodies) {
        if (excludeBody && body == excludeBody) continue;
        glm::vec3 pos = body->getPosition();
        points.emplace_back(pos.x, pos.y, pos.z);
    }

    // Build and query the KDTree
    kdt::KDTree<BodyPoint> tree(points);
    BodyPoint queryPoint(x, y, z);
    int k = 20;
    std::vector<int> indices = tree.knnSearch(queryPoint, k);

    // Collect bodies except the excluded one
    for (int idx : indices) {
        if (idx >= 0 && idx < static_cast<int>(activeBodies.size()) && activeBodies[idx] != excludeBody) {
            resultBodies.push_back(activeBodies[idx]);
        }
    }
    return resultBodies;
}