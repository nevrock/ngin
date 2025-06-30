#ifndef GL_DATA_H
#define GL_DATA_H

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <glm/glm.hpp>

struct GLFWwindow; // <--- Add this line!

struct GlData {
    GLFWwindow* window;
    
    // textures
    unsigned int tex_g_normal;
    unsigned int tex_g_albedo;
    unsigned int tex_g_specular;
    unsigned int tex_depthmap;

    // framebuffers
    unsigned int original_fb;
    unsigned int forward_fb;
    unsigned int deferred_fb;
    unsigned int ssr_fb;

    // matrices (temp)
    glm::mat4 view;
    glm::mat4 projection;

    // debug
    bool debug_enabled;

    ~GlData() {
    }
};

#endif
