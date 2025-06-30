#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <iostream>
#include <memory> // For std::unique_ptr

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/debug/logger.h>

#include <ngin/util/math.h>

#include <ngin/render/gl/data.h>
#include <ngin/render/data.h>

#include <ngin/ngin.h>

#include <stb_image.h>

class GlContext {
public:

    GlContext(const std::string& title, RenderData& render_data, GlData& gl_data) : render_data_(render_data), gl_data_(gl_data) {
        logger_ = new Logger("GlContext");

        // --- GLFW and Window Initialization ---
        // Moved from the old static `create` method.
        logger_->info("Initializing GlContext...", 0);
        
        int screen_width = render_data.screen_width;
        int screen_height = render_data.screen_height;

        last_x_ = static_cast<float>(screen_width) / 2.0f;
        last_y_ = static_cast<float>(screen_height) / 2.0f;

        if (!glfwInit()) {
            logger_->info("Failed to initialize GLFW", 1);
            // In a constructor, it's better to throw an exception on failure.
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

        gl_data_.window = glfwCreateWindow(screen_width, screen_height, title.c_str(), NULL, NULL);
        if (!gl_data_.window) {
            logger_->info("Failed to create GLFW window", 1);
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        glfwMakeContextCurrent(gl_data_.window);
        glfwSetInputMode(gl_data_.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // --- Store `this` pointer in the window ---
        // This is the key to connecting C-style GLFW callbacks to our C++ class instance.
        glfwSetWindowUserPointer(gl_data_.window, this);

        // --- Set GLFW Callbacks ---
        // These now point to static wrapper functions.
        glfwSetFramebufferSizeCallback(gl_data_.window, framebuffer_size_callback_wrapper);
        glfwSetCursorPosCallback(gl_data_.window, mouse_callback_wrapper);
        glfwSetScrollCallback(gl_data_.window, scroll_callback_wrapper);

        // --- GLAD Initialization ---
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            logger_->info("Failed to initialize GLAD", 1);
            glfwDestroyWindow(gl_data_.window);
            glfwTerminate();
            throw std::runtime_error("Failed to initialize GLAD");
        }
        
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glfwSwapInterval(1); // Enable V-Sync
        logger_->info("GlContext initialized successfully.", 0);

        set_cursor_enabled(true);
    }

    ~GlContext() {
        logger_->info("Destroying GlContext...", 0);
        if (gl_data_.window) {
            glfwDestroyWindow(gl_data_.window);
        }
        glfwTerminate();
        // unique_ptr for logger_ handles its own deletion.
        delete logger_;
    }
    
    GlContext(const GlContext&) = delete;
    
    GlContext& operator=(const GlContext&) = delete;

    void set_framebuffer(unsigned int fbo, bool is_clear = false) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (is_clear) {
            clear(true);
        }
    }
    void set_texture(unsigned int set_texture, int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, set_texture);
    }

    bool should_close() {
        return glfwWindowShouldClose(gl_data_.window);
    }
    void swap() {
        glfwSwapBuffers(gl_data_.window);
        glfwPollEvents();
    }  
    
    void update_time() {
        float currentTime = static_cast<float>(glfwGetTime());
        
        render_data_.time_delta = currentTime - render_data_.time_last;
        render_data_.time_last = currentTime;
        render_data_.time += render_data_.time_delta;
        
        if (timer_ > 0.1f) {
            render_data_.mouse_offset_x = MathUtil::lerp(render_data_.mouse_offset_x, 0.0f, render_data_.time_delta * 5.0f);
            render_data_.mouse_offset_y = MathUtil::lerp(render_data_.mouse_offset_y, 0.0f, render_data_.time_delta * 5.0f);
        }
        timer_ += render_data_.time_delta;
    }
    void process_input() {
        if (glfwGetKey(gl_data_.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(gl_data_.window, true);
            return;
        }

        float axesX = 0.0f; // Local to this function now
        float axesY = 0.0f;

        if (glfwGetKey(gl_data_.window, GLFW_KEY_W) == GLFW_PRESS) axesY = 1.0f;
        if (glfwGetKey(gl_data_.window, GLFW_KEY_S) == GLFW_PRESS) axesY = -1.0f;
        if (glfwGetKey(gl_data_.window, GLFW_KEY_A) == GLFW_PRESS) axesX = -1.0f;
        if (glfwGetKey(gl_data_.window, GLFW_KEY_D) == GLFW_PRESS) axesX = 1.0f;

        render_data_.axes_x = axesX;
        render_data_.axes_y = axesY;
    }
    
    void set_blend(bool is_transparent) {
        if (is_transparent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    void set_cull(bool is_cull, bool is_front=false) {
        if (is_cull) {
            glEnable(GL_CULL_FACE);
            glCullFace(is_front ? GL_FRONT : GL_BACK);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
    void set_depth(bool is_depth) {
        if (is_depth) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL); // Note: This was LEQUAL in your code, keeping it consistent.
        }
    }
    void set_viewport(int width, int height) {
        glViewport(0, 0, width, height);
    }
    void set_cursor_enabled(bool enabled) {
        glfwSetInputMode(gl_data_.window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    void set_custom_cursor(const std::string& imagePath, int hotspotX, int hotspotY) {
        GLFWimage image;
        if (!load_image(imagePath, image)) {
            logger_->info("Failed to load cursor image: " + imagePath, 0);
            return;
        }

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
        free_image(image); // Free the stb_image data immediately after cursor creation

        if (!cursor) {
            logger_->info("Failed to create custom cursor", 0);
            return;
        }

        glfwSetCursor(gl_data_.window, cursor);
    }
    void clear(bool is_clear_color=false) {
        if (is_clear_color)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

private:
    GlData& gl_data_;
    RenderData& render_data_;
    Logger* logger_;

    float last_x_ = 0.0f;
    float last_y_ = 0.0f;
    float timer_ = 0.0f;
    bool first_mouse_ = true;

    bool load_image(const std::string& path, GLFWimage& image) {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &image.width, &image.height, &channels, STBI_rgb_alpha);
        if (!data) {
            logger_->info("stb_image failed to load: " + path, 1);
            return false;
        }
        image.pixels = data;
        return true;
    }
    void free_image(GLFWimage& image) {
        if (image.pixels) {
            stbi_image_free(image.pixels);
            image.pixels = nullptr;
        }
    }
    void on_framebuffer_size_event(int width, int height) {
        glViewport(0, 0, width, height);
        render_data_.screen_width = width;
        render_data_.screen_height = height;
    }
    void on_mouse_event(double xposIn, double yposIn) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        float screen_height = render_data_.screen_height;

        render_data_.mouse_x = xpos;
        render_data_.mouse_y = screen_height-ypos;

        if (first_mouse_) {
            last_x_ = xpos;
            last_y_ = ypos;
            first_mouse_ = false;
        }

        float xoffset = xpos - last_x_;
        float yoffset = last_y_ - ypos; // reversed since y-coordinates go from bottom to top

        last_x_ = xpos;
        last_y_ = ypos;

        render_data_.mouse_offset_x = xoffset;
        render_data_.mouse_offset_y = yoffset;
        
        timer_ = 0.0f;
    }
    void on_scroll_event(double xoffset, double yoffset) {
        render_data_.scroll_y = static_cast<float>(yoffset);
    }
    static void framebuffer_size_callback_wrapper(GLFWwindow* window, int width, int height) {
        GlContext* self = static_cast<GlContext*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->on_framebuffer_size_event(width, height);
        }
    }
    static void mouse_callback_wrapper(GLFWwindow* window, double xposIn, double yposIn) {
        GlContext* self = static_cast<GlContext*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->on_mouse_event(xposIn, yposIn);
        }
    }
    static void scroll_callback_wrapper(GLFWwindow* window, double xoffset, double yoffset) {
        GlContext* self = static_cast<GlContext*>(glfwGetWindowUserPointer(window));
        if (self) {
            self->on_scroll_event(xoffset, yoffset);
        }
    }
};

#endif // CONTEXT_H
