#ifndef RENDERER_H
#define RENDERER_H

#include <ngin/render/gl/context.h>
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
        if (logger_) {
            delete logger_;
        }
        if (context_) {
            delete context_;
        }
    }

    void setup() {
        logger_ = new ngin::debug::Logger("RenderManager");

        setup_render_data();
        setup_render_passes();

        context_ = new GlContext("ngin", render_data_, gl_data_);
        logger_->info("RenderManager setup");
    }
    void setup_render_data() {
        std::tuple<std::string, bool> resource_path = FileUtil::get_resource_path("data/render.atl");

        Atlas data;
        data.read(std::get<0>(resource_path));

        // Use get with default value directly
        render_data_.screen_width = 1280;
        render_data_.screen_height = 720;
        render_data_.screen_width = *data.get<int>("screen.width", &render_data_.screen_width);
        render_data_.screen_height = *data.get<int>("screen.height", &render_data_.screen_height);

        logger_->info("RenderManager setup render data with screen width: " + std::to_string(render_data_.screen_width) + " and height: " + std::to_string(render_data_.screen_height));
    }
    void setup_render_passes() {
        
    }
    void update_early() {
        context_->update_time();
        context_->process_input();
    }
    void update_late() {
        context_->swap();
    }
    void cleanup() {
        logger_->info("RenderManager cleanup");
        if (context_) {
            delete context_;
            context_ = nullptr; // Set to nullptr after deletion
        }
    }

    bool should_close() {
        bool should_close = context_->should_close();
        if (should_close) {
            logger_->info("RenderManager should close");
        }
        return should_close;
    }

private:
    ngin::debug::Logger* logger_ = nullptr;
    GlContext* context_ = nullptr;
    RenderData render_data_;
    GlData gl_data_;
};

} // namespace render
} // namespace ngin

#endif