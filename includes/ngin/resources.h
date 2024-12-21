#ifndef RESOURCES_S_H
#define RESOURCES_S_H

#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <stb_image.h>

#include <ngin/utils/fileutils.h>
#include <ngin/collections/nevf.h>
#include <ngin/constants.h>
#include <ngin/log.h>

// data
#include <ngin/data/shader_data.h>
#include <ngin/data/mesh_data.h>

#if defined(_MSC_VER)
    #include <io.h>
#endif
#include <stdio.h>

class Resources {
public:
    static void init() {
        Log::console("!!!   resources started   !!!");
        // need to load in resources

        shaderManifest_ = loadNevf("manifest.nevf", "shaders/");
        meshManifest_ = loadNevf("manifest.nevf", "meshes/");
    }

    static Nevf shaderManifest_;
    static Nevf meshManifest_;

    // resources, data you can load from files
    static Nevf loadNevf(const std::string& name, const std::string& prefix = "nevf/") {
        Nevf n;
        std::string filePath = fixFilePath(prefix, name);
        n.read(FileUtils::getResourcePath(filePath));
        //n.print();
        return n;
    }
    static std::shared_ptr<ShaderData> loadShaderData(const std::string& name)
    {
        Nevf shaderManifestData = shaderManifest_.getC<Nevf>(name, Nevf());

        std::string vertexLocation = shaderManifestData.getC<std::string>("vertex", "");
        std::string fragmentLocation = shaderManifestData.getC<std::string>("fragment", "");
        std::string geometryLocation = shaderManifestData.getC<std::string>("geometry", "");
        std::string includeLocation = shaderManifestData.getC<std::string>("include", "");
        std::string headerLocation = shaderManifestData.getC<std::string>("header", ngin::SHADER_INCLUDE_MANDATORY);

        Log::console("loading shader! " + name);

        std::string vShaderFilePath = FileUtils::getResourcePath("shaders/" + vertexLocation + ".nvtx");
        std::string fShaderFilePath = FileUtils::getResourcePath("shaders/" + fragmentLocation + ".nfrg");
        std::string gShaderFilePath = FileUtils::getResourcePath("shaders/" + geometryLocation + ".ngeo");
        std::string iShaderFilePath = FileUtils::getResourcePath("shaders/" + includeLocation + ".ninc");
        std::string hShaderFilePath = FileUtils::getResourcePath("shaders/" + headerLocation + ".ninc");

        const char* vShaderPath = FileUtils::doesPathExist(vShaderFilePath) ? vShaderFilePath.c_str() : nullptr;
        const char* fShaderPath = FileUtils::doesPathExist(fShaderFilePath) ? fShaderFilePath.c_str() : nullptr;
        const char* gShaderPath = FileUtils::doesPathExist(gShaderFilePath) ? gShaderFilePath.c_str() : nullptr;
        const char* iShaderPath = FileUtils::doesPathExist(iShaderFilePath) ? iShaderFilePath.c_str() : nullptr;
        const char* hShaderPath = FileUtils::doesPathExist(hShaderFilePath) ? hShaderFilePath.c_str() : nullptr;

        return std::make_shared<ShaderData>(name, vShaderPath, fShaderPath, hShaderPath, gShaderPath, iShaderPath);
    }
    
    static std::shared_ptr<MeshData> loadMeshData(const std::string& name) {
        Nevf meshManifestData = meshManifest_.getC<Nevf>(name, Nevf());
        //meshManifest_.print();
        //meshManifestData.print();
        std::string location = meshManifestData.getC<std::string>("location", "");

        Log::console("loading mesh! " + name + ", at location: " + location);

        std::string meshFilePath = FileUtils::getResourcePath("meshes/" + location + ".nmsh");

        Log::console("mesh filepath: " + meshFilePath);

        Nevf n;
        n.read(meshFilePath);

        n.print();

        std::vector<float> verticesFlattened = n.getC<std::vector<float>>("vertices", std::vector<float>());
        std::string meshName = n.getC<std::string>("name", "");

        return std::make_shared<MeshData>(meshName, verticesFlattened);
    }

private:
    // static std::vector<std::shared_ptr<Shader>> shaders_;

    static std::string fixFilePath(const std::string& prefix, const std::string& name) {
        std::string filePath = name;
        if (filePath.find(prefix) == std::string::npos) {
            filePath = prefix + filePath;
        } else {
            filePath = filePath.substr(filePath.find(prefix));
        }
        if (filePath.find(".nevf") == std::string::npos) {
            filePath += ".nevf";
        }
        return filePath;
    }
};

#endif // RESOURCES_H
