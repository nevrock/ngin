#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <snorri/dict.h>  // Include the JSON library header

#include <snorri/log.h>

struct ModelData {
public:
    std::string name;
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<Texture> textures;
    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;

    Mesh* getMesh(int index) {
        if (index < meshes.size()) {
            return meshes[index].get();
        } return nullptr;
    }

    auto& getBoneInfoMap() { return boneInfoMap; }
	int& getBoneCount() { return boneCounter; }

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
