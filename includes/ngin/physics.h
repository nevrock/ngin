#ifndef PHYSICS_H
#define PHYSICS_H

#include <atomic>
#include <memory>
#include <functional>
#include <thread>

#include <ngin/scene.h>


class Physics {
public:
    static void init() {
        std::cout << "!!!   physics started   !!!" << std::endl;
    }
    static void connectScene(Scene* scene) {
        scene_ = scene;
    }
    static void run() {
        running = true;
        std::thread physicsThread([&]() {
            float lastFramePhysics = static_cast<float>(glfwGetTime());  // Initialize lastFramePhysics here
            while (running) {
                float currentFrame = static_cast<float>(glfwGetTime());
                float deltaTimePhysics = currentFrame - lastFramePhysics;

                if (deltaTimePhysics > fixedPhysicsStep_) {
                    lastFramePhysics = currentFrame;
                    if (scene_)
                        scene_->executePass("physics");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Simulate the passage of time between frames
            }
        });
        physicsThread.detach(); // Detach the thread
    }
    static std::atomic<bool> running;
private:
    // Constructor, Copy constructor, and Assignment operator are private to prevent multiple instances
    Physics() {}
    Physics(const Physics&) = delete;
    Physics& operator=(const Physics&) = delete;

    static Scene* scene_;

    static constexpr float fixedPhysicsStep_ = 0.015f; // Declare as constexpr

};

#endif // PHYSICS_H