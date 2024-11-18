#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include <string>

// Define the namespace
namespace ngin {
    // Define constants within the namespace
    constexpr  unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    constexpr  unsigned int AMOUNT_INSTANCES = 1000;

    constexpr  unsigned int SCREEN_WIDTH = 1024, SCREEN_HEIGHT = 576;

    constexpr  unsigned int RENDER_LAYER_UI = 4;
    constexpr  unsigned int RENDER_LAYER_DIRECTIONAL_DEPTH = 0;
    constexpr   unsigned int RENDER_LAYER_POINT_DEPTH = 1;
    constexpr  unsigned int RENDER_LAYER_THRESHOLD_SHADOWS = 2;
    constexpr  unsigned int RENDER_LAYER_THRESHOLD_UI = 4;

    inline const char* SHADER_INCLUDE_MANDATORY = "nev";
}

#endif // SNORRI_CONSTANTS_H
