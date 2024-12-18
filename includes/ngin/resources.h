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

        shaderManifest_ = loadNevf("shaders/manifest.nevf");
        meshManifest_ = loadNevf("meshes/manifest.nevf");
    }

    // resources, data you can load from files
    static Nevf loadNevf(const std::string& name) {
        Nevf n;
        std::string filePath = fixFilePath("nevf/", name);
        n.read(FileUtils::getResourcePath(filePath));
        //n.print();
        return n;
    }
    static std::shared_ptr<ShaderData> loadShaderData(const std::string& name)
    {
        Nevf shaderManifestData = shaderManifest_.getC<Nevf>(name, Nevf());
        std::string location = shaderManifestData.getC<std::string>("location", "");

        Log::console("loading shader! " + name + ", at location: " + location);
        
        std::string vShaderFilePath = FileUtils::getResourcePath("shaders/" + location + ".nvtx");
        std::string fShaderFilePath = FileUtils::getResourcePath("shaders/" + location + ".nfrg");
        std::string gShaderFilePath = FileUtils::getResourcePath("shaders/" + location + ".ngeo");
        std::string iShaderFilePath = FileUtils::getResourcePath("shaders/" + location + ".ninc");
        std::string hShaderFilePath = FileUtils::getResourcePath("shaders/" + std::string(ngin::SHADER_INCLUDE_MANDATORY) + ".ninc");

        if (FileUtils::doesPathExist(gShaderFilePath)) {
            if (FileUtils::doesPathExist(iShaderFilePath)) {
                return std::make_shared<ShaderData>(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), gShaderFilePath.c_str(), iShaderFilePath.c_str());
            } else {
                return std::make_shared<ShaderData>(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), gShaderFilePath.c_str(), nullptr);
            }
        } else {
            if (FileUtils::doesPathExist(iShaderFilePath)) {
                return std::make_shared<ShaderData>(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), nullptr, iShaderFilePath.c_str());
            } else {
                return std::make_shared<ShaderData>(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), nullptr, nullptr);
            }
        }    
        return nullptr;
    }
    static std::shared_ptr<MeshData> loadMeshData(const std::string& name) {
        Nevf meshManifestData = meshManifest_.getC<Nevf>(name, Nevf());
        std::string location = meshManifestData.getC<std::string>("location", "");

        Log::console("loading mesh! " + name + ", at location: " + location);

        std::string meshFilePath = FileUtils::getResourcePath("meshes/" + location + ".nmsh");

        Nevf n;
        n.read(meshFilePath);

        return std::make_shared<MeshData>(n);
    }

private:
    // static std::vector<std::shared_ptr<Shader>> shaders_;
    static Nevf shaderManifest_;
    static Nevf meshManifest_;

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
