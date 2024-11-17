#ifndef SURFACE_SDF_H
#define SURFACE_SDF_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <nev/object.h>

#include <nev/surface_data.h>

class SurfaceSdf : public Component
{
public:

    SurfaceSdf(IObject* parentObj) : Component(parentObj) {}

    SurfaceData getSurfaceData() {
        return SurfaceData {
            glm::vec4(parent->getWorldPosition(), 1.0), // Assuming getPoint().getPosition() returns glm::vec3
            glm::vec4(0.0, 0.0, 0.0, 1.0), // Correctly initialized as a vec4
            glm::vec4(0.5, 0.5, 0.5, 1.0), // Assuming scale needs to be vec4, otherwise use vec3
            glm::vec4(0.5, 1.0, 0.5, 1.0), // Diffuse color
            0, // shapeType
            0, // blendType
            0.0, // blendStrength
            0  // outline
        };
    }

    void loadFromDict(const Dict& d) override;

private:
 
};

#endif
