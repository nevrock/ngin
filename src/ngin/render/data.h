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
};

#endif
