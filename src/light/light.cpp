#include "light.h"
#include <glm/glm.hpp>
#include <iostream>
#include <ngin/resources.h>
#include <limits>
#include <algorithm>
#undef max

Light* Light::mainLight = nullptr;
std::vector<Light*> Light::pointLights;

Light::Light(Object* parent) 
    : Component(parent) {

}

Light* Light::getClosestPointLight(glm::vec3 pos) {
    if (pointLights.size() == 0) {
        return nullptr;

    }
    Light* closestLight = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (Light* pointLight : pointLights) {
        float distance = glm::distance(pointLight->getPosition(), pos);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestLight = pointLight;
        }
    }
    return closestLight;
}
Light::~Light() {
    if (isPoint) {
        auto it = std::find(pointLights.begin(), pointLights.end(), this);
        if (it != pointLights.end()) {
            pointLights.erase(it);
        }
    }
}

void Light::loadFromNevf(const Nevf& d) {
    if (d.getC<bool>("is_main", false)) {
        setMainLight(this);
        isMain = true;
    } else {
        isMain = false;
        if (d.getC<bool>("is_point", false)) {
            isPoint = true;
            pointLights.push_back(this);
        }
    }
}

void Light::update() {
}

void Light::updatePreRender(const unsigned int index, Shader& shader) {
    if (!isMain) return;
    
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;

    glm::vec3 lightDir = getPoint().getFrontDirection();  // Use a normalized direction vector for the light

    float nearPlane = 1.0f, farPlane = 45.0f;  // Adjust these to ensure your scene fits within the light's frustum
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);

    glm::vec3 arbitraryCenter = glm::vec3(5.0f);  // You might need to adjust this to be the center of your scene
    lightView = glm::lookAt(arbitraryCenter - lightDir * 10.0f, arbitraryCenter, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    shader.setMat4("M_LIGHT_SPACE", lightSpaceMatrix);
    shader.setVec3("LIGHT_DIRECTION", lightDir);
    shader.setVec3("LIGHT_POS", getParent()->getPoint().getPosition());
    shader.setFloat("LIGHT_FAR_PLANE", farPlane);
    shader.setFloat("LIGHT_NEAR_PLANE", nearPlane);
}
void Light::updateShadows(Shader& shader) {
    glm::vec3 position = getPoint().getPosition();

    float nearPlane = 1.0f;
    float farPlane  = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)ngin::SHADOW_WIDTH / (float)ngin::SHADOW_HEIGHT, nearPlane, farPlane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

    for (unsigned int i = 0; i < 6; ++i)
        shader.setMat4("A_M_POINT_SHADOWS[" + std::to_string(i) + "]", shadowTransforms[i]);
    
    shader.setVec3("POINT_LIGHT_POS", position);
}

Light* Light::getMainLight() {
    return mainLight;
}
void Light::setMainLight(Light* light) {
    mainLight = light;
}

glm::vec3 Light::getPosition() {
    return getPoint().getPosition();
}
glm::vec3 Light::getDirection() {
    return getPoint().getFrontDirection();
}



bool light_registered = []() {
    Log::console("light registering now");
    Object::registerComponent("light", [](Object* parent) {
        return std::make_shared<Light>(parent);
    });
    return true;
}();
