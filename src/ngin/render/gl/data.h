#ifndef GL_DATA_H
#define GL_DATA_H

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <glm/glm.hpp>

struct GLFWwindow; // <--- Add this line!

struct GlData {
    GLFWwindow* window;

    int screen_width;
    int screen_height;

    int view_width;
    int view_height;

    float time;
    float time_delta;
    float time_last;

    float mouse_x;
    float mouse_y;
    float mouse_offset_x;
    float mouse_offset_y;
    float scroll_y;

    float axes_x;
    float axes_y;

    // matrices (temp)
    glm::mat4 view;
    glm::mat4 projection;

    // debug
    bool debug_enabled;

    ~GlData() {
    }
};

#endif
