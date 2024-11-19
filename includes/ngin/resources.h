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

#include <ngin/gl/shader.h>
#include <ngin/gl/texture_2d.h>
#include <ngin/utils/fileutils.h>
#include <ngin/log.h>
#include <ngin/gl/mesh.h>
#include <ngin/gui/font.h>
#include <ngin/constants.h>
#include <ngin/scene/model_loader.h>

#if defined(_MSC_VER)
    #include <io.h>
#endif
#include <stdio.h>

class Resources {
public:

    inline static std::map<std::string, Shader>    shaders;
    inline static std::map<std::string, std::unique_ptr<Texture2D>> textures; // Changed to use std::unique_ptrc
    inline static std::map<std::string, std::unique_ptr<ModelData>> models; // Changed to use std::unique_ptrc
    inline static std::map<std::string, std::unique_ptr<Font>> fonts; // Changed to use std::unique_ptrc

    static void clear()
    {
        // (properly) delete all shaders	
        for (const auto& iter : shaders) {  // Using 'const auto&' to avoid copying
            glDeleteProgram(iter.second.ID);
        }

        // Properly delete all textures
        for (const auto& iter : textures) {  // Using 'const auto&' to avoid copying
            if (iter.second) {  // Check if the unique_ptr actually points to an object
                glDeleteTextures(1, &iter.second->id);
            }
        }

        textures.clear();
        shaders.clear();
        models.clear();
        fonts.clear();
    }
    
    static std::string getResourcePath(const std::string& path) 
    {
        return FileUtils::getResourcePath(path);
    }
    
    static Shader getShader(const std::string& name) {
        auto it = shaders.find(name);
        if (it == shaders.end()) {
            std::cerr << "Shader not found: " << name << ". Loading default shader." << std::endl;
            // Optionally, load a default shader here or throw an exception
            loadShader(name);  // Potentially risky if name still doesn't correspond to a valid shader file
            it = shaders.find(name);
            if (it == shaders.end()) {
                throw std::runtime_error("Failed to load shader: " + name);
            }
        }
        return it->second;
    }
    static Shader loadShader(const std::string& name)
    {
        Log::console("load shader! " + name);
        std::string vShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".vs");
        std::string fShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".fs");
        std::string gShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".gs");
        std::string iShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".include");
        std::string hShaderFilePath = FileUtils::getResourcePath("shaders/" + std::string(ngin::SHADER_INCLUDE_MANDATORY) + ".include");

        if (FileUtils::doesPathExist(gShaderFilePath)) {
            if (FileUtils::doesPathExist(iShaderFilePath)) {
                shaders[name] = Shader(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), gShaderFilePath.c_str(), iShaderFilePath.c_str());
            } else {
                shaders[name] = Shader(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), gShaderFilePath.c_str(), nullptr);
            }
        } else {
            if (FileUtils::doesPathExist(iShaderFilePath)) {
                shaders[name] = Shader(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), nullptr, iShaderFilePath.c_str());
            } else {
                shaders[name] = Shader(vShaderFilePath.c_str(), 
                    fShaderFilePath.c_str(), hShaderFilePath.c_str(), nullptr, nullptr);
            }
        }    
        return shaders[name];
    }

    static Texture2D& getTexture(const std::string& name) {
        auto it = textures.find(name);
        if (it == textures.end()) {
            std::cerr << "Texture not found: " << name << ". Loading texture." << std::endl;
            return loadTexture(name);
        }
        return *(it->second);
    }
    static Texture2D& loadTexture(const std::string& name) {
        Log::console("load texture! " + name);
        std::string path = FileUtils::getResourcePath("textures/" + name + ".png");
        auto& texture = textures[name]; // Create a new unique_ptr entry if it does not exist
        if (!texture) {
            texture = std::make_unique<Texture2D>(); // Create a new Texture2D if not already loaded
        }
        if (loadTextureFromFile(path.c_str(), *texture)) {
            return *texture;
        } else {
            throw std::runtime_error("Failed to load texture: " + name);
        }
    }
    static bool loadTextureFromFile(const char* path, Texture2D& texture) {
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

    static Font& getFont(const std::string& name) {
        auto it = fonts.find(name);
        if (it == fonts.end()) {
            std::cerr << "Font not found: " << name << ". Loading font." << std::endl;
            return loadFont(name);
        }
        return *(it->second);
    }
    static Font& loadFont(const std::string& name) {
        Log::console("load font! " + name);
        std::string path = FileUtils::getResourcePath("fonts/" + name + ".ttf");
        auto& font = fonts[name]; // Create a new unique_ptr entry if it does not exist
        if (!font) {
            font = std::make_unique<Font>(); // Create a new Texture2D if not already loaded
        }
        if (loadFontFromFile(path.c_str(), *font)) {
            return *font;
        } else {
            throw std::runtime_error("Failed to load font: " + name);
        }
    }
    static bool loadFontFromFile(const char* path, Font& font) {
        font.generate(std::string(path), 24);
        return true;
    }

    static ModelData* getModel(const std::string& name) {
        // Log::console("get model! " + name);
        auto it = models.find(name);
        if (it == models.end()) {
            std::cerr << "model not found: " << name << "." << std::endl;
            return nullptr;
        }
        return it->second.get();
    }
    static ModelData& loadModelPrimitive(const std::string name) {

        auto& model = models[name]; // Create a new unique_ptr entry if it does not exist
        if (!model) {
            model = std::make_unique<ModelData>(); // Create a new Animation if not already loaded
            
            std::vector<Texture> textures;

            Nevf d;
            d.read(FileUtils::getResourcePath("nevf/models/" + name + ".nevf"));
            int texIndex = 2;
            if (d.contains("textures")) {
                for (const auto& textureName : d.getC<std::vector<std::string>>("textures", std::vector<std::string>{""})) {
                    Log::console("resources loading texture for mesh prim! " + std::string(textureName));
                    Texture texture;
                    texture.id = texIndex,
                    texture.name = textureName;
                    textures.push_back(texture);

                    texIndex += 1;
                }
            }


            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            if (name == "plane") {
                vertices.reserve(6);
                indices.reserve(6);
                float planeVertices[] = {
                    // positions            // normals         // texcoords
                    25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,  25.0f,  0.0f,
                    -25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,   0.0f,  0.0f,
                    -25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,   0.0f, 25.0f,

                    25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
                    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
                    25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f 

                };
                for (uint32_t i = 0; i < 6; ++i) {
                    Vertex vertex{
                        glm::vec3(planeVertices[i*8], planeVertices[i*8 + 1], planeVertices[i*8 + 2]),
                        glm::vec3(planeVertices[i*8 + 3], planeVertices[i*8 + 4], planeVertices[i*8 + 5]),
                        glm::vec2(planeVertices[i*8 + 6], planeVertices[i*8 + 7]),
                        glm::vec3(1.0f, 1.0f, 1.0f)
                    };
                    vertices.push_back(vertex);
                }
                for (uint32_t i = 0; i < 6; ++i) {
                    indices.push_back(i);
                }
            } else if (name == "cube") {
                vertices.reserve(36);
                indices.reserve(36);
                float cubeVertices[] = {
                    // back face
                    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
                    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                    // front face
                    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                    1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                    // left face
                    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                    // right face
                    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                    // bottom face
                    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                    1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                    // top face
                    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                    1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                    1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
                };

                for (uint32_t i = 0; i < 36; ++i) {
                    Vertex vertex{
                        glm::vec3(cubeVertices[i*8], cubeVertices[i*8 + 1], cubeVertices[i*8 + 2]),
                        glm::vec3(cubeVertices[i*8 + 3], cubeVertices[i*8 + 4], cubeVertices[i*8 + 5]),
                        glm::vec2(cubeVertices[i*8 + 6], cubeVertices[i*8 + 7]),
                        glm::vec3(1.0f, 1.0f, 1.0f)
                    };
                    vertices.push_back(vertex);
                }
                for (uint32_t i = 0; i < 36; ++i) {
                    indices.push_back(i);
                }

            } else if (name == "quad") {
                vertices.reserve(4);
                indices.reserve(6);
                float cubeVertices[] = {
                    // back face
                    -0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 
                    -0.5f, 0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
                    0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 
                    0.5f,  -0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f
                };

                for (uint32_t i = 0; i < 4; ++i) {
                    Vertex vertex{
                        glm::vec3(cubeVertices[i*8], cubeVertices[i*8 + 1], cubeVertices[i*8 + 2]),
                        glm::vec3(cubeVertices[i*8 + 3], cubeVertices[i*8 + 4], cubeVertices[i*8 + 5]),
                        glm::vec2(cubeVertices[i*8 + 6], cubeVertices[i*8 + 7]),
                        glm::vec3(1.0f, 1.0f, 1.0f)
                    };
                    vertices.push_back(vertex);
                }
                indices.push_back(2);
                indices.push_back(1);
                indices.push_back(0);
                indices.push_back(3);
                indices.push_back(2);
                indices.push_back(0);
            }
            auto newMesh = std::make_unique<Mesh>();
            newMesh->vertices = std::move(vertices);
            newMesh->indices = std::move(indices);
            newMesh->textures = std::move(textures);
            newMesh->setupMesh();  
            model.get()->meshes.push_back(std::move(newMesh));
        } 

        return *model;
    }
};

#endif // RESOURCES_H
