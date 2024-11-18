#ifndef TIME_H
#define TIME_H

#include <chrono>

class Time {
public:
        
    static void initialize() {
        startTime = std::chrono::high_resolution_clock::now();
        lastUpdate = std::chrono::high_resolution_clock::now();
        lastPhysicsUpdate = std::chrono::high_resolution_clock::now();
    }

    static bool update() {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastUpdate).count();
        if (timeSinceLastUpdate >= delta) {
            lastUpdate = now;
            since = std::chrono::duration_cast<std::chrono::duration<float>>(now - startTime).count();
            return true;
        }
        return false;
    }

    static bool updatePhysics() {
        auto now = std::chrono::high_resolution_clock::now();
        timeSinceLastPhysicsUpdate = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastPhysicsUpdate).count();
        if (timeSinceLastPhysicsUpdate >= deltaPhysics) {
            lastPhysicsUpdate = now;
            return true;
        }
        return false;
    }

    static float delta;        // Fixed time step for updates in seconds
    static float deltaPhysics; // Fixed physics time step in seconds
    static float since;        // Time since startup in seconds
    static float timeSinceLastPhysicsUpdate; // Time since last physics update

private:
    static std::chrono::high_resolution_clock::time_point startTime;
    static std::chrono::high_resolution_clock::time_point lastUpdate; // Time of the last update
    static std::chrono::high_resolution_clock::time_point lastPhysicsUpdate; // Time of the last physics update
};

std::chrono::high_resolution_clock::time_point Time::startTime;
std::chrono::high_resolution_clock::time_point Time::lastUpdate;
std::chrono::high_resolution_clock::time_point Time::lastPhysicsUpdate;
float Time::delta = 0.005f;  // 30 updates per second
float Time::deltaPhysics = 0.016f;  // Physics update rate, approx. 60 updates per second
float Time::since = 0.0f;
float Time::timeSinceLastPhysicsUpdate = 0.0f;

#endif // TIME_H
