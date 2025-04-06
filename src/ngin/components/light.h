#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>

#include <ngin/drawer.h>    

class Light : public IDrawer {
public:
    Light(const std::string name, const Lex& lex, IObject* parent)
        : IDrawer(name, lex, parent) {
        
        std::vector<std::string> shaders = lex.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::registerDrawer(shader, *this);
        }
    }
    ~Light() {
        std::vector<std::string> shaders = lex_.getC<std::vector<std::string>>("shaders", {});
        for (const auto& shader : shaders) {
            Drawer::unregisterDrawer(shader, *this);
        }
    }

    void init() override {
        // Implementation of init method
    }

    void launch() override {
        // Implementation of launch method
    }

    void update() override {
        // Implementation of update method
    }

    void updateLate() override {
        // Implementation of updateLate method
    }

    void prep(ShaderData& shader) override {
        // Implementation of prep method
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;

        glm::vec3 lightDir = -getPointTransform()->getUp();  // Use a normalized direction vector for the light
        glm::vec3 lightPos = getPointTransform()->getWorldPosition();  // Use the light's position
        float nearPlane = 1.0f, farPlane = 45.0f;  // Adjust these to ensure your scene fits within the light's frustum
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);

        glm::vec3 arbitraryCenter = glm::vec3(0.0f);  // You might need to adjust this to be the center of your scene
        lightView = glm::lookAt(arbitraryCenter - lightDir * 10.0f, arbitraryCenter, glm::vec3(0.0, 0.0, 1.0)); // Adjusted up vector
        lightSpaceMatrix = lightProjection * lightView;

        glm::vec3 lightColor = lex_.getC<glm::vec3>("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Default to white light
        shader.setVec3("lightColor", lightColor);

        float lightIntensity = lex_.getC<float>("lightIntensity", 0.5f); // Default intensity is 1.0
        shader.setFloat("lightIntensity", lightIntensity);

        //shader.setMat4("M_LIGHT_SPACE", lightSpaceMatrix);
        shader.setVec3("lightDir", lightDir);
        shader.setVec3("worldLightDir", lightDir);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    }

    void draw(ShaderData& shader) override {
        // Implementation of draw method
    }

private:
    // ...existing code...
};

#endif // LIGHT_H
