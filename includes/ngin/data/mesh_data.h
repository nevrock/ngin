#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector> // Ensure you include the vector header

#include <ngin/data/i_data.h>   
#include <ngin/collections/nevf.h>
#include <ngin/log.h>

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

class MeshData : public IData {
public:
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;
    unsigned int VBO, EBO;

    glm::mat4 localTransform;

    glm::mat4 getMeshMatrix() {
        return localTransform;
    }

    void execute() override {
        // cook this data so it is stable for remaining passes
    }

    std::string getName() override { return name_; } 

    MeshData() {}

    MeshData(std::string name, Nevf data) : name_(name) {

        Log::console("creating MeshData: " + name);
        Nevf verticeDict = data.getC<Nevf>("vertices", Nevf());
        Log::console("vertices: " + std::to_string(verticeDict.length()));

        std::vector<int> triangles = data.getC<std::vector<int>>("triangles", std::vector<int>());
        Log::console("triangles: " + std::to_string(triangles.size()));

        vertices.reserve(verticeDict.length());
        for (const auto& kvp : verticeDict) {
            Nevf vertexData = std::any_cast<Nevf>(kvp.second);
            Vertex vertex;
            vertex.position = vertexData.getVec("position", glm::vec3(0.0f));
            vertex.normal = vertexData.getVec("normal", glm::vec3(0.0f));
            vertex.uv = vertexData.getVec2("uv", glm::vec2(0.0f));
            vertex.color = vertexData.getVec("color", glm::vec3(1.0f));
            auto boneIDs = vertexData.getC<std::vector<int>>("bone_ids", std::vector<int>(MAX_BONE_INFLUENCE, 0));
            auto weights = vertexData.getC<std::vector<float>>("weights", std::vector<float>(MAX_BONE_INFLUENCE, 0.0f));
            std::copy(boneIDs.begin(), boneIDs.end(), vertex.m_BoneIDs);
            std::copy(weights.begin(), weights.end(), vertex.m_Weights);
            vertices.push_back(vertex);
        }

        Log::console("vertices initialized: " + std::to_string(vertices.size()));

        indices.reserve(triangles.size());
        for (int i = 0; i < triangles.size(); i++) {
            indices.push_back(triangles[i]);
        }

        Log::console("indices initialized: " + std::to_string(indices.size())); 

        setupMesh();
    }

    ~MeshData() {
        std::cout << "destroying Mesh with VAO: " << VAO << std::endl;
        // Proper cleanup
        //if (glIsVertexArray(VAO)) {
        //    glDeleteVertexArrays(1, &VAO);
        //}
        //if (glIsBuffer(VBO)) {
        //    glDeleteBuffers(1, &VBO);
        //}
        //if (glIsBuffer(EBO)) {
        //    glDeleteBuffers(1, &EBO);
        //}
    }

    void render() {
        // std::cout << "Rendering Mesh with VAO: " << VAO << std::endl;
        // draw mesh

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // initializes all the buffer objects/arrays
    void setupMesh() {
        Log::console("setting up Mesh... ");

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        std::cout << "setting up Mesh with VAO: " << VAO << std::endl;
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

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
    std::string name_;
}; // Added missing semicolon here at the end of the class definition

#endif // MESH_H
