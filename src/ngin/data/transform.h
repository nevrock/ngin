#ifndef TRANSFORM_DATA_H
#define TRANSFORM_DATA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

struct TransformData {    
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    void from_atlas(Atlas* data) {
        std::vector<float>* position_data = data->get<std::vector<float>>("position");
        if (position_data) {
            position = glm::vec3(
                position_data->at(0),
                position_data->at(1),
                position_data->at(2)
            );
        }
        std::vector<float>* rotation_data = data->get<std::vector<float>>("rotation");
        if (rotation_data) {
            rotation = glm::vec3(
                rotation_data->at(0),
                rotation_data->at(1),
                rotation_data->at(2)
            );
        }
        std::vector<float>* scale_data = data->get<std::vector<float>>("scale");
        if (scale_data) {
            scale = glm::vec3(
                scale_data->at(0),
                scale_data->at(1),
                scale_data->at(2)
            );
        }
    }
};

#endif // TRANSFORM_DATA_H