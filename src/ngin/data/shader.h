#ifndef SHADER_DATA_H
#define SHADER_DATA_H

#include <glm/glm.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>


struct ShaderData {
    std::string header_path = "";
    std::string vertex_path = "";
    std::string fragment_path = "";
    std::string geometry_path = "";

    Atlas* attributes = nullptr;

    void from_data(Atlas& data, ngin::debug::Logger* logger = nullptr) {
        std::string* name;
        name = data.get<std::string>("name", name);

        std::string* header_path;
        header_path = data.get<std::string>("header", header_path);

        std::string* vertex_path;
        vertex_path = data.get<std::string>("vertex", vertex_path);

        std::string* fragment_path;
        fragment_path = data.get<std::string>("fragment", fragment_path);

        std::string* geometry_path;
        geometry_path = data.get<std::string>("geometry", geometry_path);

        attributes = data.get<Atlas>("attributes", attributes);
    }
};

#endif // SHADER_DATA_H