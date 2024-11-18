#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <ngin/dict.h>  // Include the JSON library header

#include <ngin/log.h>

struct ModelData {
public:
    std::string name;
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<Texture> textures;

    Mesh* getMesh(int index) {
        if (index < meshes.size()) {
            return meshes[index].get();
        } return nullptr;
    }

    void render() {
        for (auto& mesh : meshes) {
            mesh->render();
        }
    }
    void setName(std::string name) { 
        this->name = name;
    }
};

#endif // MODEL_LOADER_H
