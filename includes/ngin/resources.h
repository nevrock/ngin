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
#include <ngin/data/texture_data.h>

#if defined(_MSC_VER)
    #include <io.h>
#endif
#include <stdio.h>

class Resources {
public:
    inline static std::map<std::string, std::unique_ptr<MeshData>> meshes_; // Changed to use std::unique_ptrc
    inline static std::map<std::string, std::unique_ptr<ShaderData>> shaders_;
    inline static std::map<std::string, std::unique_ptr<TextureData>> textures_;

    inline static Nevf shaderManifest_;
    inline static Nevf meshManifest_;
    inline static Nevf textureManifest_;

    static void init() {
        Log::console("!!!   resources started   !!!");
        // need to load in resources

        shaderManifest_ = loadNevf("manifest.nevf", "shaders/");
        meshManifest_ = loadNevf("manifest.nevf", "meshes/");
        textureManifest_ = loadNevf("manifest.nevf", "textures_/");
    }
    static void terminate() {
        Log::console("!!!   resources terminated   !!!");
        // need to unload resources
        for (auto& [name, mesh] : meshes_) {
            mesh.reset();
        }
        for (auto& [name, shader] : shaders_) {
            shader.reset();
        }
        for (auto& [name, texture] : textures_) {
            texture.reset();
        }
    }

    // resources, data you can load from files
    static Nevf loadNevf(const std::string& name, const std::string& prefix = "nevf/") {
        Nevf n;
        std::string filePath = fixFilePath(prefix, name);
        n.read(FileUtils::getResourcePath(filePath));
        //n.print();
        return n;
    }
    static ShaderData& getShaderData(const std::string& name) {
        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            std::cerr << "shader not found: " << name << ", requires loading" << std::endl;
            loadShaderData(name);  // Potentially risky if name still doesn't correspond to a valid shader file
            it = shaders_.find(name);
            if (it == shaders_.end()) {
                throw std::runtime_error("failed to load shader: " + name);
            }
            return *(it->second);
        }
        return *(it->second);
    }
    static void loadShaderData(const std::string& name)
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

        shaders_[name] = std::make_unique<ShaderData>(name, vShaderPath, fShaderPath, hShaderPath, gShaderPath, iShaderPath);
    }
    static void unloadShaderData(const std::string& name) {
        auto it = shaders_.find(name);
        if (it != shaders_.end()) {
            shaders_.erase(it);
        }
    }
    
    static MeshData& getMeshData(const std::string& name) {
        auto it = meshes_.find(name);
        if (it == meshes_.end()) {
            std::cerr << "mesh not found: " << name << ". Loading default mesh." << std::endl;
            loadMeshData(name);  // Potentially risky if name still doesn't correspond to a valid mesh file
            it = meshes_.find(name);
            if (it == meshes_.end()) {
                throw std::runtime_error("Failed to load mesh: " + name);
            }
            return *(it->second);
        }
        return *(it->second);
    }
    static void loadMeshData(const std::string& name) {

        Nevf meshManifestData = meshManifest_.getC<Nevf>(name, Nevf());
        std::string location = meshManifestData.getC<std::string>("location", "");
        Log::console("loading mesh! " + name + ", at location: " + location);
        std::string meshFilePath = FileUtils::getResourcePath("meshes/" + location + ".nmsh");
        Log::console("mesh filepath: " + meshFilePath);

        Nevf n;
        n.read(meshFilePath);
        n.print();

        std::string meshName = n.getC<std::string>("name", "");

        meshes_[name] = std::make_unique<MeshData>(meshName, n);
    }
    static void unloadMeshData(const std::string& name) {
        auto it = meshes_.find(name);
        if (it != meshes_.end()) {
            meshes_.erase(it);
        }
    }

    static TextureData& getTextureData(const std::string& name) {
        auto it = textures_.find(name);
        if (it == textures_.end()) {
            std::cerr << "Texture not found: " << name << ". Loading texture." << std::endl;
            return loadTextureData(name);
        }
        return *(it->second);
    }
    static TextureData& loadTextureData(const std::string& name) {
        Log::console("load texture! " + name);
        Nevf textureManifestData = textureManifest_.getC<Nevf>(name, Nevf());
        std::string path = textureManifestData.getC<std::string>("location", "");

        auto& texture = textures_[name]; // Create a new unique_ptr entry if it does not exist
        if (!texture) {
            texture = std::make_unique<TextureData>(); // Create a new TextureData if not already loaded
        }
        if (loadTextureFromFile(path.c_str(), *texture)) {
            return *texture;
        } else {
            throw std::runtime_error("Failed to load texture: " + name);
        }
    }
    static void unloadTextureData(const std::string& name) {
        auto it = textures_.find(name);
        if (it != textures_.end()) {
            textures_.erase(it);
        }
    }
    static bool loadTextureFromFile(const char* path, TextureData& texture) {
        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (!data) {
            std::cerr << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
            return false;
        }

        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        texture.internalFormat = format;
        texture.imageFormat = format;
        texture.wrapS = (format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        texture.wrapT = (format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        texture.filterMin = GL_LINEAR_MIPMAP_LINEAR;
        texture.filterMax = GL_LINEAR;
        texture.generate(width, height, data);
        
        stbi_image_free(data);
        return true;
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
