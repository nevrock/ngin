#pragma once

#include <ngin/object.h>
#include <glm/glm.hpp>
#include <ngin/constants.h>
#include <vector>

class Light : public Component {
public:
    static Light* getMainLight();
    static void setMainLight(Light* light);  // Static method to set the main camera
    static std::vector<Light*> pointLights;  // Static list of all point lights
    static Light* getClosestPointLight(glm::vec3 pos);  // Static method to get the closest point light

    Light(Object* parent);
    ~Light();  // Destructor to remove point lights from the list

    void loadFromNevf(const Nevf& d) override;
    void update() override;
    void updatePreRender(const unsigned int index, Shader& shader) override;
    void updateShadows(Shader& shader);

    glm::vec3 getPosition();
    glm::vec3 getDirection();

private:
    static Light* mainLight;  // Static pointer to the main camera

    bool isMain;
    bool isPoint;
};
