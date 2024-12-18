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
#include <fmt/core.h>


struct Vertex {
    // position
    glm::vec3 position;
    // color
    glm::vec3 normal;
    // texCoords
    glm::vec2 uv;
    // color
    glm::vec3 color;
};

class MeshData : public IData {
public:

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO;
    unsigned int VBO, EBO;

    void execute() override {}

    std::string getName() override { return name_; } 

    MeshData() {}
    MeshData(const std::string& name) : name_(name) {
        // Default constructor

    }
    MeshData(const Nevf& data) : name_(data.getC<std::string>("name", "")) {
        std::vector<float> verticesFlattened = data.getC<std::vector<float>>("vertices", std::vector<float>());
        std::vector<int> triangles = data.getC<std::vector<int>>("triangles", std::vector<int>());

        // Assuming each vertex has 8 attributes: position (3), normal (3), uv (2)
        for (size_t i = 0; i < verticesFlattened.size(); i += 8) {
            Vertex vertex;
            vertex.position = glm::vec3(verticesFlattened[i], verticesFlattened[i + 1], verticesFlattened[i + 2]);
            vertex.normal = glm::vec3(verticesFlattened[i + 3], verticesFlattened[i + 4], verticesFlattened[i + 5]);
            vertex.uv = glm::vec2(verticesFlattened[i + 6], verticesFlattened[i + 7]);
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // Default color, can be modified as needed
            vertices.push_back(vertex);
        }

        indices = std::vector<unsigned int>(triangles.begin(), triangles.end());

        setupMesh();
    }
    MeshData(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;
        Log::console(fmt::format("constructing mesh with {1} vertices and {0} indices", vertices.size(), indices.size()));
        setupMesh();
    }
    ~MeshData() {
        Log::console(fmt::format("Destroying Mesh with VAO: {}", VAO));
        // Proper cleanup
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void render() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        Log::console(fmt::format("Setting up Mesh with VAO: ", VAO));
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        // Vertex Colors
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        // ids
		//glEnableVertexAttribArray(4);
		//glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
		// weights
		//glEnableVertexAttribArray(4);
		//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        

        glBindVertexArray(0); // Unbind VAO
    }

private:
    std::string name_;

};

#endif