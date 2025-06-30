#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <iomanip> // Required for std::setw, std::setprecision

// Third-party libraries for windowing and OpenGL loading
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ngin/job/system_stealer.h>
#include <ngin/job/handle.h>
#include <ngin/job/render_job.h>

// Forward declarations...
GLFWwindow* g_window = nullptr;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
bool init_window();
unsigned int create_shader_program();
unsigned int create_triangle_vao();

// A predefined list of all job types to ensure stable printing order.
const std::vector<JobType> all_job_types = {
    JobType::Physics,
    JobType::Animation,
    JobType::AI,
    JobType::TransformSync,
    JobType::RenderCommandGeneration,
    JobType::AssetLoading,
    JobType::Other
};

// MODIFIED: Diagnostic function now accepts FPS to print on the same line.
void print_job_system_diagnostics(const ngin::jobs::JobSystem& job_system, int fps) {
    const auto snapshot = job_system.get_diagnostics_snapshot();
    const int total_jobs = snapshot.total_pending;

    std::stringstream ss;
    
    // Use an ANSI escape code to clear the entire line (\x1b[2K) and then
    // use a carriage return (\r) to move the cursor to the beginning.
    // This is far more reliable across different terminals than using \r alone.
    ss << "\x1b[2K\r";

    ss << "FPS - " << std::left << std::setw(4) << fps << " | ";
    ss << "Job Total - " << std::left << std::setw(5) << total_jobs << " | Job Breakdown - ";
    
    // Iterate over the predefined list of all job types for a stable layout.
    for (const auto type : all_job_types) {
        int count = 0;
        // Find the count for the current type in the snapshot
        auto it = snapshot.type_counts.find(type);
        if (it != snapshot.type_counts.end()) {
            count = it->second;
        }

        float percentage = (total_jobs > 0) ? (static_cast<float>(count) * 100.0f) / total_jobs : 0.0f;
        
        // Always print the type, even if its percentage is zero.
        ss << to_string(type) << ": " << std::fixed << std::setprecision(1) << std::setw(4) << percentage << "% ";
    }
            
    // The extra padding is no longer necessary because \x1b[2K clears the whole line.
    std::cout << ss.str() << std::flush;
}


// --- Shader Sources (No changes) ---
const char* vertex_shader_source = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 ourColor;
    uniform mat4 model;
    void main() {
       gl_Position = model * vec4(aPos, 1.0);
       ourColor = aColor;
    }
)glsl";

const char* fragment_shader_source = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec3 ourColor;
    void main() {
       FragColor = vec4(ourColor, 1.0f);
    }
)glsl";

// main function and other helpers (no changes)...
int main() {
    if (!init_window()) return -1;
    
    glfwSetKeyCallback(g_window, key_callback);
    ngin::jobs::JobSystem job_system;

    unsigned int shader_program = create_shader_program();
    unsigned int triangle_vao = create_triangle_vao();
    GLint model_loc = glGetUniformLocation(shader_program, "model");
    RenderCommandQueue render_command_queue;
    const int NUM_OBJECTS = 5000;
    std::vector<float> object_angles(NUM_OBJECTS, 0.0f);
    std::vector<float> object_speeds(NUM_OBJECTS, 0.0f);
    std::vector<float> object_x_pos(NUM_OBJECTS, 0.0f);
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> speed_dist(0.5f, 2.0f);
    std::uniform_real_distribution<float> pos_dist(-0.95f, 0.95f);
    for(int i = 0; i < NUM_OBJECTS; ++i) {
        object_speeds[i] = speed_dist(rng);
        object_x_pos[i] = pos_dist(rng);
    }

    // Variables for controlling diagnostic print frequency
    auto last_diag_print_time = std::chrono::steady_clock::now();
    const auto diag_print_interval = std::chrono::milliseconds(250);

    // NEW: Variables for FPS calculation
    int frame_counter = 0;
    int fps = 0;
    auto last_frame_time = std::chrono::steady_clock::now();


    std::cout << "Starting main loop with " << NUM_OBJECTS << " objects. Press ESC to exit." << std::endl;
    while (!glfwWindowShouldClose(g_window)) {
        // NEW: Delta time and FPS counter logic
        auto current_time = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_frame_time).count();
        last_frame_time = current_time;
        frame_counter++;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --- PHASE 1: AI Job Submission ---
        std::vector<std::function<void()>> ai_tasks;
        ai_tasks.reserve(NUM_OBJECTS);
        for (int i = 0; i < NUM_OBJECTS; ++i) {
            ai_tasks.push_back([i, &object_angles, &object_speeds, delta_time]() {
                object_angles[i] += object_speeds[i] * delta_time;
            });
        }
        // Submit all AI jobs. The submit_jobs function will wait for any specified dependencies.
        // Since there are no dependencies here, it will submit immediately.
        JobHandle ai_handle = job_system.submit_jobs(ai_tasks, JobType::AI);

        // --- PHASE 2: Render Command Generation Job Submission ---
        // These jobs *depend* on the AI jobs. The submit_jobs call will internally
        // call wait_for(ai_handle) before pushing these render generation tasks to the queue.
        std::vector<std::function<void()>> render_gen_tasks;
        render_gen_tasks.reserve(NUM_OBJECTS);
        for (int i = 0; i < NUM_OBJECTS; ++i) {
             render_gen_tasks.push_back([i, shader_program, triangle_vao, &render_command_queue, &object_angles, &object_x_pos]() {
                // By the time this task starts executing, the AI job for 'i' will have completed
                Mat4 model;
                float angle = object_angles[i];
                float x = object_x_pos[i];
                float y = std::sin(angle * 5.0f + x * 2.0f) * 0.5f;
                float scale = 0.05f;
                float cosA = cos(angle);
                float sinA = sin(angle);
                model.elements[0] = cosA * scale;
                model.elements[1] = sinA * scale;
                model.elements[4] = -sinA * scale;
                model.elements[5] = cosA * scale;
                model.elements[12] = x;
                model.elements[13] = y;
                RenderCommand cmd = { shader_program, triangle_vao, 3, model };
                render_command_queue.push(cmd);
            });
        }
        // Submit RenderCommandGeneration jobs, making them *depend* on the AI jobs.
        // The main thread will block here until ai_handle is complete.
        JobHandle render_gen_handle = job_system.submit_jobs(render_gen_tasks, JobType::RenderCommandGeneration, {ai_handle});

        // Check if it's time to update and print diagnostics
        if (current_time - last_diag_print_time > diag_print_interval) {
            fps = static_cast<int>(frame_counter / std::chrono::duration<float>(current_time - last_diag_print_time).count());
            print_job_system_diagnostics(job_system, fps);
            last_diag_print_time = current_time;
            frame_counter = 0;
        }

        // Wait for *all* render command generation jobs to complete before drawing.
        // This implicitly ensures AI jobs are also done.
        job_system.wait_for(render_gen_handle);

        std::vector<RenderCommand> commands_to_execute;
        render_command_queue.pop_all(commands_to_execute);
        glUseProgram(shader_program);
        for (const auto& cmd : commands_to_execute) {
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, cmd.model_matrix.elements);
            glBindVertexArray(cmd.vao_id);
            glDrawArrays(GL_TRIANGLES, 0, cmd.vertex_count);
        }
        glBindVertexArray(0);
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    std::cout << std::endl; 
    std::cout << "Shutting down..." << std::endl;
    job_system.shutdown();
    glDeleteVertexArrays(1, &triangle_vao);
    glDeleteProgram(shader_program);
    glfwTerminate();

    return 0;
}

// --- Helper Function Implementations (no changes) ---
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

bool init_window() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    g_window = glfwCreateWindow(1280, 720, "Job System Renderer Example", NULL, NULL);
    if (g_window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int create_shader_program() {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

unsigned int create_triangle_vao() {
    float vertices[] = {
        // positions      // colors
        -1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return VAO;
}