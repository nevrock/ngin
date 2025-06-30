#ifndef DRAWER_DATA_H
#define DRAWER_DATA_H

#include <string>


struct DrawerData {
    // debug
    bool debug_enabled;

    DrawerData() : debug_enabled(false) {
    }
    ~DrawerData() {
    }

    std::string mesh;
    std::string material;
};

#endif
