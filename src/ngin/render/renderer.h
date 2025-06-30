#ifndef RENDERER_H
#define RENDERER_H

#include <ngin/debug/logger.h>
#include <ngin/util/file.h>
#include <ngin/render/gl/context.h>
#include <ngin/render/data.h>
#include <ngin/render/pass.h>
#include <ngin/ngin.h>

#include <iostream>
#include <string>
#include <sstream> // Required for std::stringstream
#include <vector>
#include <tuple>

class Renderer {

public:
    Renderer() : render_data_(NGIN::render()) {
    }
    ~Renderer() {
        if (logger_) {
            delete logger_;
        }
        if (context_) {
            delete context_;
        }
    }

    void setup() {
        logger_ = new Logger("Renderer");

        setup_render_data();
        setup_render_passes();

        context_ = new GlContext("ngin", render_data_, gl_data_);

        std::vector<std::string> shader_assets = NGIN::assets().get_asset_names_for_bucket("shader");
        for (const auto& shader_asset : shader_assets) {
            logger_->info("Loading shader asset: " + shader_asset);
            NGIN::assets().load_asset("shader", shader_asset);
        }
        
        logger_->info("Renderer setup");
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

        logger_->info("Renderer setup render data with screen width: " + std::to_string(render_data_.screen_width) + " and height: " + std::to_string(render_data_.screen_height));
    }
    void setup_render_passes() {
        
    }
    void update() {
        context_->update_time();
        context_->process_input();

        context_->swap();
    }
    void cleanup() {
        logger_->info("Renderer cleanup");
        if (context_) {
            delete context_;
            context_ = nullptr; // Set to nullptr after deletion
        }
    }

    bool should_close() {
        bool should_close = context_->should_close();
        if (should_close) {
            logger_->info("Renderer should close");
        }
        return should_close;
    }

private:
    Logger* logger_ = nullptr;
    GlContext* context_ = nullptr;
    std::vector<RenderPass*> render_passes_;

    RenderData& render_data_;
    GlData gl_data_;
};

#endif