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

private:
    // static std::vector<std::shared_ptr<Shader>> shaders_;
};

#endif // RESOURCES_H
