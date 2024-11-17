#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <vector>
#include <memory>

#include <array>
#include <iostream>

class BodyPoint : public std::array<double, 3> {
    public:
        static const int DIM = 3;
        BodyPoint() {}
        BodyPoint(double x, double y, double z) {
            (*this)[0] = x;
            (*this)[1] = y;
            (*this)[2] = z;
        }
};

class Body;

class PhysicsManager {
public:
    static std::vector<std::shared_ptr<Body>> activeBodies;

    static std::vector<std::shared_ptr<Body>> getClosest(double x, double y, double z, std::shared_ptr<Body> excludeBody = nullptr);

    static void registerBody(const std::shared_ptr<Body>& body);
    static void unregisterBody(const std::shared_ptr<Body>& body);
private:
};

#endif // PHYSICS_MANAGER_H
