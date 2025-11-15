#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include <vector>

struct RenderData {
    // debug
    bool debug_enabled;

    RenderData() : debug_enabled(false) {
    }
    ~RenderData() {
    }

    // textures
    unsigned int tex_g_position;
    unsigned int tex_g_normal;
    unsigned int tex_g_albedo;

    unsigned int tex_g_aa_position;
    unsigned int tex_g_aa_normal;
    unsigned int tex_g_aa_albedo;

    unsigned int tex_ssao_albedo;
    unsigned int tex_ssao_blur;

    unsigned int tex_g_specular;
    unsigned int tex_depthmap;

    // framebuffers
    unsigned int original_fb;
    unsigned int forward_fb;
    unsigned int forward_aa_fb;
    unsigned int ssao_fb;
    unsigned int ssao_blur_fb;
    unsigned int deferred_fb;
    unsigned int ssr_fb;

    // renderbuffer
    unsigned int depth_rb;
};

#endif
