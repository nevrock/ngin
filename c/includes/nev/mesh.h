#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector> // Ensure you include the vector header

#define MAX_BONE_INFLUENCE 4
#define MAX_TEXTURES 4

struct Vertex {
    // position
    glm::vec3 position;
    // color
    glm::vec3 normal;
    // texCoords
    glm::vec2 uv;
    // color
    glm::vec3 color;
    //bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string name;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;
    unsigned int VBO, EBO;

    std::vector<Mesh> shapeKeys;  // A vector of Mesh objects, each representing a shape key

    glm::mat4 localTransform;

    glm::mat4 getMeshMatrix() {
        return localTransform;
    }

    Mesh() {}

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        std::cout << "Constructing Mesh with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
        setupMesh();
    }

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;
        std::cout << "Constructing Mesh with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
        setupMesh();
    }

    ~Mesh() {
        std::cout << "Destroying Mesh with VAO: " << VAO << std::endl;
        // Proper cleanup
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void render() {
        // std::cout << "Rendering Mesh with VAO: " << VAO << std::endl;
        // draw mesh

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void addShapeKey(const Mesh& shapeKey) {
        shapeKeys.push_back(shapeKey);
    }

    // initializes all the buffer objects/arrays
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        std::cout << "Setting up Mesh with VAO: " << VAO << std::endl;
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
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
		// weights
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        

        glBindVertexArray(0); // Unbind VAO
    }

private:
    
}; // Added missing semicolon here at the end of the class definition

#endif // MESH_H
