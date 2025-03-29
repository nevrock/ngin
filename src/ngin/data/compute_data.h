#ifndef COMPUTE_DATA_H
#define COMPUTE_DATA_H

#include <glad/glad.h>
#include <ngin/data/i_data.h>
#include <ngin/log.h>
#include <fstream>
#include <sstream>
#include <string>

class ComputeData : public IData {
public:
    unsigned int ID;

    ComputeData() {}
    ComputeData(const std::string name, const char* computePath) : name_(name), compPath_(computePath) {
        load();
    }

    void execute() override {}

    std::string getName() override { return name_; }

    void load() {
        std::string computeCode;
        std::ifstream cShaderFile;

        cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            if (compPath_) {
                cShaderFile.open(compPath_);
                std::stringstream cShaderStream;
                cShaderStream << cShaderFile.rdbuf();
                cShaderFile.close();
                computeCode = cShaderStream.str();
            }
        } catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::COMPUTE_SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            std::cerr << "Compute Path: " << compPath_ << std::endl;
        }
        const char* cShaderCode = computeCode.c_str();
        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(compute);

        Log::console("Compute shader loaded: " + name_, 1);
    }

    void use() {
        glUseProgram(ID);
    }

    void dispatch(unsigned int num_groups_x, unsigned int num_groups_y, unsigned int num_groups_z) {
        use();
        glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    const char* compPath_;
    const std::string name_;

    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                Log::error("Shader compilation error of type: " + type + "\n" + std::string(infoLog));
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                Log::error("Shader program linking error of type: " + type + "\n" + std::string(infoLog));
            }
        }
    }
};

#endif
