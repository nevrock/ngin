#ifndef RENDER_JOB_H
#define RENDER_JOB_H

// Include the thread-safe queue from your original files.
#include "ngin/job/queue.h"
#include <vector>

/**
 * @struct Mat4
 * @brief A simple 4x4 matrix struct to avoid needing a full math library for this example.
 * The job system will calculate transformations and store them in this struct.
 */
struct Mat4 {
    // Stored in column-major order for direct use with OpenGL.
    float elements[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
};

/**
 * @struct RenderCommand
 * @brief Represents a single, self-contained command to be executed by the renderer.
 * Jobs will generate these commands in parallel, and the main thread will execute them.
 */
struct RenderCommand {
    unsigned int shader_program_id;
    unsigned int vao_id;
    int vertex_count;
    Mat4 model_matrix; // Each command stores the pre-calculated model matrix for an object.
};

/**
 * @typedef RenderCommandQueue
 * @brief A type alias for a thread-safe queue that will hold the generated RenderCommands.
 */
using RenderCommandQueue = ThreadableQueue<RenderCommand>;

#endif // RENDER_JOB_H
