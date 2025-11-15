#ifndef RENDERER_H
#define RENDERER_H

#include <ngin/render/gl/context.h>
#include <ngin/render/context.h>
#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>
#include <ngin/util/file.h>

#include <iostream>
#include <string>
#include <sstream> // Required for std::stringstream
#include <vector>
#include <tuple>

namespace ngin {
namespace render {

class RenderManager {

public:
    RenderManager() {
    }
    ~RenderManager() {
        logger_->info("RenderManager cleanup");
        if (logger_) {
            delete logger_;
        }
    }

    void setup() {
        logger_ = new ngin::debug::Logger("RenderManager");

        setup_render_data();
        setup_render_passes();

        gl_context_ = new GlContext("ngin", gl_data_);
        render_context_ = new RenderContext(render_data_, *gl_context_);
        logger_->info("RenderManager setup");
    }
    void setup_render_data() {
        std::tuple<std::string, bool> resource_path = FileUtil::get_resource_path("data/render.atl");

        Atlas data;
        data.read(std::get<0>(resource_path));

        // Use get with default value directly
        gl_data_.screen_width = 1280;
        gl_data_.screen_height = 720;
        gl_data_.screen_width = *data.get<int>("screen.width", &gl_data_.screen_width);
        gl_data_.screen_height = *data.get<int>("screen.height", &gl_data_.screen_height);

        logger_->info("RenderManager setup render data with screen width: " + std::to_string(gl_data_.screen_width) + " and height: " + std::to_string(gl_data_.screen_height));
    }
    void setup_render_passes() {
        
    }
    void update_early() {
        gl_context_->update_time();
        gl_context_->process_input();
    }
    void update_late() {
        gl_context_->swap();
    }
    void cleanup() {
        // logger_->info("RenderManager cleanup");
        // if (gl_context_) {
        //     delete gl_context_;
        //     gl_context_ = nullptr; // Set to nullptr after deletion
        // }
        if (gl_context_) {
            delete gl_context_;
        }
        if (render_context_) {
            delete render_context_;
        }
    }

    bool should_close() {
        bool should_close = gl_context_->should_close();
        if (should_close) {
            logger_->info("RenderManager should close");
        }
        return should_close;
    }

private:
    ngin::debug::Logger* logger_ = nullptr;

    GlContext* gl_context_ = nullptr;
    RenderContext* render_context_ = nullptr;

    RenderData render_data_;
    GlData gl_data_;
};

} // namespace render
} // namespace ngin

#endif