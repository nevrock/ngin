#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include <string>
#include <iostream>
#include <memory> // For std::unique_ptr

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/debug/logger.h>
#include <ngin/util/math.h>
#include <ngin/render/data.h>
#include <ngin/render/gl/context.h>

#include <stb_image.h>

class RenderContext {
public:

    RenderContext(RenderData& data, GlContext& gl_context) : data_(data), gl_context_(gl_context) {
        logger_ = new ngin::debug::Logger("RenderContext");
        logger_->info("Creating RenderContext...");
    }

    ~RenderContext() {
        logger_->info("Destroying RenderContext...");
        delete logger_;
    }
    
private:
    ngin::debug::Logger* logger_;    
    RenderData& data_;
    GlContext& gl_context_;
};

#endif // RENDER_CONTEXT_H
