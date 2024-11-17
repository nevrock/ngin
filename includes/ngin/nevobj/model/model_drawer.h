// model_drawer.h

#ifndef MODEL_DRAWER_H
#define MODEL_DRAWER_H

#include <map>
#include <string>
#include <vector>
#include <snorri/constants.h>

class Model;
class Shader;

struct DrawerData {
    std::map<std::string, std::vector<Model*>> modelsLoadedByLayer;
};

class ModelDrawer {
public:
    static void addModel(const std::string& key, unsigned int layer, Model* data);
    static bool removeModel(const std::string& key, unsigned int layer, Model* data);

    static void init();
    static void render(const unsigned int index, Shader& shader);
    static void renderInstances(const unsigned int index, Shader& shader);

private:
    static std::map<unsigned int, DrawerData> data; // Declaration only
    static unsigned int buffer;
};

#endif
