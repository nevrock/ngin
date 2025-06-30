#ifndef GL_SHADER_DATA_H
#define GL_SHADER_DATA_H

#include <fstream> // Add this include directive
#include <sstream> // Add this include directive
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ngin/data/shader.h>

class GlShaderData {
public:
    GlShaderData(const std::string name, ShaderData& data) : name_(name), data_(data) {
    }

    void load() {
        std::string vertex_code;
        std::string fragment_code;
        std::string geometry_code;
        std::ifstream header_shader_file;
        std::ifstream vertex_shader_file;
        std::ifstream fragment_shader_file;
        std::ifstream geometry_shader_file;

        vertex_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fragment_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        geometry_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            if (data_.vertex_path != "") {
                vertex_shader_file.open(data_.vertex_path);
                std::stringstream vertex_shader_stream;
                vertex_shader_stream << vertex_shader_file.rdbuf();
                vertex_shader_file.close();
                vertex_code = vertex_shader_stream.str();
            }

            if (data_.fragment_path != "") {
                fragment_shader_file.open(data_.fragment_path);
                std::stringstream fragment_shader_stream;
                fragment_shader_stream << fragment_shader_file.rdbuf();
                fragment_shader_file.close();
                fragment_code = fragment_shader_stream.str();
            }

            if (data_.geometry_path != "") {
                geometry_shader_file.open(data_.geometry_path);
                std::stringstream geometry_shader_stream;
                geometry_shader_stream << geometry_shader_file.rdbuf();
                geometry_shader_file.close();
                geometry_code = geometry_shader_stream.str();
            }

            if (data_.header_path != "") {
                header_shader_file.open(data_.header_path);
                std::stringstream header_shader_stream;
                header_shader_stream << header_shader_file.rdbuf();
                header_shader_file.close();
                fragment_code = header_shader_stream.str() + "\n" + fragment_code;
                vertex_code = header_shader_stream.str() + "\n" + vertex_code;
                if (data_.geometry_path != "")
                    geometry_code = header_shader_stream.str() + "\n" + geometry_code;
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            std::cerr << "Vertex Path: " << data_.vertex_path << std::endl;
            std::cerr << "Fragment Path: " << data_.fragment_path << std::endl;
            if (data_.geometry_path != "") {
                std::cerr << "Geometry Path: " << data_.geometry_path << std::endl;
            }
        }
        const char* vertex_shader_code = vertex_code.c_str();
        const char * fragment_shader_code = fragment_code.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment, geometry;
        // Vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader_code, NULL);
        glCompileShader(vertex);
        check_compilation_errors(vertex, "VERTEX");
        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_shader_code, NULL);
        glCompileShader(fragment);
        check_compilation_errors(fragment, "FRAGMENT");

        // Shader program
        id_ = glCreateProgram();
        glAttachShader(id_, vertex);
        glAttachShader(id_, fragment);

        if(data_.geometry_path != "") {
            const char * gShaderCode = geometry_code.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            check_compilation_errors(geometry, "GEOMETRY");

            glAttachShader(id_, geometry);
        }

        glLinkProgram(id_);
        check_compilation_errors(id_, "PROGRAM");

        // Delete shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(data_.geometry_path != "") {
            glDeleteShader(geometry);
        }
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(id_); 
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void set_bool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(id_, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void set_int(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(id_, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void set_float(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(id_, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void set_vec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]); 
    }
    void set_vec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(id_, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void set_vec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]); 
    }
    void set_vec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(id_, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void set_vec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]); 
    }
    void set_vec4(const std::string &name, float x, float y, float z, float w) 
    { 
        glUniform4f(glGetUniformLocation(id_, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void set_mat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void set_mat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void set_mat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }


private:
    unsigned int id_;
    const std::string name_;

    ShaderData& data_;

    void check_compilation_errors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            }
        }
    }
};

#endif