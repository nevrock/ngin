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

    static Nevf loadNevf(const std::string& name) {
        Nevf n;
        n.read(FileUtils::getResourcePath("nevf/" + name + ".nevf"));
        n.print();
        return n;
    }
};

#endif // RESOURCES_H
