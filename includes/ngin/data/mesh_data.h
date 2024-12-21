#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector> // Ensure you include the vector header

#include <ngin/data/i_data.h>
#include <ngin/log.h>
#include <ngin/collections/nevf.h>


class MeshData : public IData {
public:

    void execute() override {}

    std::string getName() override { return name_; } 

    MeshData() {}
    MeshData(std::string name, std::vector<float> vertices) 
        : name_(name), vertices_(vertices) {
        Log::console("constructing mesh with " + std::to_string(this->vertices_.size()) + " vertices");
        setupMesh();
    }
    ~MeshData() {
        Log::console("destroying mesh with vao_: " + std::to_string(vao_));
        // Proper cleanup
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
    }
    void render() {
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    void setupMesh() {
        Log::console("setting up vao...");
        glGenVertexArrays(1, &vao_);
        Log::console("setting up vbo...");
        glGenBuffers(1, &vbo_);
        
        Log::console("gen mesh with vao_: " + std::to_string(vao_));

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_STATIC_DRAW);

        Log::console("setting up mesh with vao_: " + std::to_string(vao_));
        
        glBindVertexArray(vao_);

        // Vertex Positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0); // Unbind vao_
    }

private:
    std::string name_;
    std::vector<float> vertices_;
    unsigned int vao_;
    unsigned int vbo_;
};

#endif