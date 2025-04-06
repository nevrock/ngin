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
#include <ngin/lex.h>
#include <ngin/constants.h>
#include <ngin/log.h>

// data
#include <ngin/data/shader_data.h>
#include <ngin/data/mesh_data.h>
#include <ngin/data/texture_data.h>
#include <ngin/data/compute_data.h>
#include <ngin/data/animation_data.h>
#include <ngin/data/font_data.h>

#if defined(_MSC_VER)
    #include <io.h>
#endif
#include <stdio.h>

class Resources {
public:
    inline static std::map<std::string, std::unique_ptr<MeshData>> meshes_; // Changed to use std::unique_ptrc
    inline static std::map<std::string, std::unique_ptr<ShaderData>> shaders_;
    inline static std::map<std::string, std::unique_ptr<TextureData>> textures_;
    inline static std::map<std::string, std::unique_ptr<ComputeData>> computeShaders_;
    inline static std::map<std::string, std::unique_ptr<AnimationData>> animations_;
    inline static std::map<std::string, std::unique_ptr<FontData>> fonts_;

    inline static Lex shaderManifest_;
    inline static Lex meshManifest_;
    inline static Lex textureManifest_;
    inline static Lex animationManifest_;

    static void init() {
        // need to load in resources

        shaderManifest_ = loadLexicon("manifest.lexf", "shader/");
        meshManifest_ = loadLexicon("manifest.lexf", "mesh/");
        textureManifest_ = loadLexicon("manifest.lexf", "texture/");
    }

    static void terminate() {
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
        for (auto& [name, computeShader] : computeShaders_) {
            computeShader.reset();
        }
    }

    // resources, data you can load from files
    static Lex loadLexicon(const std::string& name, const std::string& prefix = "data/") {
        Lex n;
        std::string filePath = fixFilePath(prefix, name);
        n.read(FileUtils::getResourcePath(filePath));
        //n.print();
        return n;
    }
    static Lex loadObject(const std::string& name) {
        return loadLexicon(name + ".nobj", "object/");
    }
    static ShaderData& getShaderData(const std::string& name) {
        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            Log::console("Shader not found: " + name + ", requires loading", 1);
            loadShaderData(name);  // Potentially risky if name still doesn't correspond to a valid shader file
            it = shaders_.find(name);
            if (it == shaders_.end()) {
                throw std::runtime_error("failed to load shader: " + name);
            }
            return *(it->second);
        }
        return *(it->second);
    }
    static void loadShaderData(const std::string& name) {
        Lex shaderManifestData = shaderManifest_.getC<Lex>(name, Lex());

        std::string vertexLocation = shaderManifestData.getC<std::string>("vertex", "");
        std::string fragmentLocation = shaderManifestData.getC<std::string>("fragment", "");
        std::string geometryLocation = shaderManifestData.getC<std::string>("geometry", "");
        std::string includeLocation = shaderManifestData.getC<std::string>("include", "");
        std::string headerLocation = shaderManifestData.getC<std::string>("header", ngin::SHADER_INCLUDE_MANDATORY);

        Log::console("Loading shader! " + name, 1);

        std::string vShaderFilePath = FileUtils::getResourcePath("shader/" + vertexLocation + ".nvtx");
        std::string fShaderFilePath = FileUtils::getResourcePath("shader/" + fragmentLocation + ".nfrg");
        std::string gShaderFilePath = FileUtils::getResourcePath("shader/" + geometryLocation + ".ngeo");
        std::string iShaderFilePath = FileUtils::getResourcePath("shader/" + includeLocation + ".ninc");
        std::string hShaderFilePath = FileUtils::getResourcePath("shader/" + headerLocation + ".ninc");

        Log::console("Vertex shader file located at: " + vShaderFilePath, 1);
        Log::console("Fragment shader file located at: " + fShaderFilePath, 1);

        const char* vShaderPath = FileUtils::doesPathExist(vShaderFilePath) ? vShaderFilePath.c_str() : nullptr;
        const char* fShaderPath = FileUtils::doesPathExist(fShaderFilePath) ? fShaderFilePath.c_str() : nullptr;
        const char* gShaderPath = FileUtils::doesPathExist(gShaderFilePath) ? gShaderFilePath.c_str() : nullptr;
        const char* iShaderPath = FileUtils::doesPathExist(iShaderFilePath) ? iShaderFilePath.c_str() : nullptr;
        const char* hShaderPath = FileUtils::doesPathExist(hShaderFilePath) ? hShaderFilePath.c_str() : nullptr;
        
        shaders_[name] = std::make_unique<ShaderData>(name, vShaderPath, fShaderPath, hShaderPath, gShaderPath, iShaderPath);

        shaders_[name]->use();

        // Set default attributes
        Lex defaults = shaderManifestData.getC<Lex>("defaults", Lex());
        for (const auto& [key, value] : defaults.data()) {
            if (value.type() == typeid(int)) {
                shaders_[name]->setInt(key, std::any_cast<int>(value));
                Log::console("Setting default attribute: " + key + " to int value " + std::to_string(std::any_cast<int>(value)), 1);
            } else if (value.type() == typeid(float)) {
                shaders_[name]->setFloat(key, std::any_cast<float>(value));
                Log::console("Setting default attribute: " + key + " to float value " + std::to_string(std::any_cast<float>(value)), 1);
            } else if (defaults.isType(key, typeid(std::vector<float>))) {
                shaders_[name]->setVec3(key, defaults.getVec(key, glm::vec3(0.0f)));
                Log::console("Setting default attribute: " + key, 1);            
            }
        }
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

        Lex meshManifestData = meshManifest_.getC<Lex>(name, Lex());
        std::string location = meshManifestData.getC<std::string>("location", "");
        Log::console("Loading mesh! " + name + ", at location: " + location, 1);
        std::string meshFilePath = FileUtils::getResourcePath("vox/" + location + ".nvox");
        Log::console("Mesh filepath: " + meshFilePath, 1);

        Lex n;
        n.read(meshFilePath);
        //n.print();

        std::string meshName = n.getC<std::string>("name", "");
        
        std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>(meshName, n);
        meshData->load();
        meshes_[name] = std::move(meshData);
    }
    static void unloadMeshData(const std::string& name) {
        auto it = meshes_.find(name);
        if (it != meshes_.end()) {
            meshes_.erase(it);
        }
    }

    static unsigned int getTextureId(const std::string& name) {
        return getTextureData(name).id;
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
        Lex textureManifestData = textureManifest_.getC<Lex>(name, Lex());
        std::string path = textureManifestData.getC<std::string>("location", "");
        auto& texture = textures_[name]; // Create a new unique_ptr entry if it does not exist
        if (!texture) {
            texture = std::make_unique<TextureData>(); // Create a new TextureData if not already loaded
        }
        if (loadTextureFromFile(FileUtils::getResourcePath("texture/" + path + ".png").c_str(), *texture)) {
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

    static FontData& getFont(const std::string& name) {
        auto it = fonts_.find(name);
        if (it == fonts_.end()) {
            std::cerr << "FontData not found: " << name << ". Loading font." << std::endl;
            return loadFont(name);
        }
        return *(it->second);
    }
    static FontData& loadFont(const std::string& name) {
        Log::console("load font! " + name);
        std::string path = FileUtils::getResourcePath("font/" + name + ".ttf");
        auto& font = fonts_[name]; // Create a new unique_ptr entry if it does not exist
        if (!font) {
            font = std::make_unique<FontData>(); // Create a new Texture2D if not already loaded
        }
        if (loadFontFromFile(path.c_str(), *font)) {
            return *font;
        } else {
            throw std::runtime_error("Failed to load font: " + name);
        }
    }
    static bool loadFontFromFile(const char* path, FontData& font) {
        font.generate(std::string(path), 16);
        return true;
    }

    static ComputeData& getComputeData(const std::string& name) {
        auto it = computeShaders_.find(name);
        if (it == computeShaders_.end()) {
            Log::error("Compute shader not found: " + name + ". Loading compute shader.", 1);
            loadComputeData(name);  // Potentially risky if name still doesn't correspond to a valid compute shader file
            it = computeShaders_.find(name);
            if (it == computeShaders_.end()) {
                throw std::runtime_error("failed to load compute shader: " + name);
            }
            return *(it->second);
        }
        return *(it->second);
    }
    static void loadComputeData(const std::string& name) {
        Lex shaderManifestData = shaderManifest_.getC<Lex>(name, Lex());

        std::string computeLocation = shaderManifestData.getC<std::string>("compute", "");

        Log::console("loading compute shader! " + name);

        std::string cShaderFilePath = FileUtils::getResourcePath("shader/" + computeLocation + ".ncmp");

        Log::console("cShaderFilePath: " + cShaderFilePath);

        const char* cShaderPath = FileUtils::doesPathExist(cShaderFilePath) ? cShaderFilePath.c_str() : nullptr;

        computeShaders_[name] = std::make_unique<ComputeData>(name, cShaderPath);

        computeShaders_[name]->use();
    }
    static void unloadComputeData(const std::string& name) {
        auto it = computeShaders_.find(name);
        if (it != computeShaders_.end()) {
            computeShaders_.erase(it);
        }
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
        if (filePath.find(".lexf") == std::string::npos) {
            size_t pos = filePath.find_last_of('.');
            if (pos != std::string::npos && filePath.substr(pos + 1).find('n') == 0) {
                return filePath;
            }
            filePath += ".lexf";
        }
        return filePath;
    }
};

#endif // RESOURCES_H
