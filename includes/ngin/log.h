// Log.h
#ifndef LOG_H
#define LOG_H

#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Log {
public:
    static void console(const std::string& logText, unsigned int indent = 0) {
        for (unsigned int i = 0; i < indent; ++i) {
            std::cout << "  "; // Print two spaces for each indentation level
        }
        std::cout << logText << std::endl;
    }
    static void error(const std::string& logText) {
        std::cout << "ERROR :: " << logText << std::endl;
    }
    static void console(const glm::vec3& vec) {
        std::cout << "vec3: (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
    }
    static void console(const glm::mat4& mat) {
        std::cout << "mat4: " << std::endl;
        const float* matPtr = glm::value_ptr(mat);
        for (int i = 0; i < 4; ++i) {
            std::cout << "[";
            for (int j = 0; j < 4; ++j) {
                std::cout << matPtr[j + i * 4];
                if (j < 3) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
private:

};

#endif // LOG_H
