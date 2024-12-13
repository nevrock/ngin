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
#include <ngin/data/shader_data.h>

#if defined(_MSC_VER)
    #include <io.h>
#endif
#include <stdio.h>

class Resources {
public:
    static void init() {
        std::cout << "!!!   resources started   !!!" << std::endl;

        // need to load in resources
    }
    static Nevf loadNevf(const std::string& name) {
        Nevf n;
        n.read(FileUtils::getResourcePath("nevf/" + name + ".nevf"));
        //n.print();
        return n;
    }
    static std::shared_ptr<ShaderData> loadShaderData(const std::string& name)
    {
        std::cout << "loading shader! " << name << std::endl;

        std::string vShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".nvs");
        std::string fShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".vfs");
        std::string gShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".vgs");
        std::string iShaderFilePath = FileUtils::getResourcePath("shaders/" + name + ".ninc");
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

private:
    // static std::vector<std::shared_ptr<Shader>> shaders_;
};

#endif // RESOURCES_H
