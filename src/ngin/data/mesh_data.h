#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include <ngin/data/i_data.h>   
#include <ngin/lex.h>
#include <ngin/log.h>

#include <ngin/data/voxa_data.h>

#define MAX_BONE_INFLUENCE 4
#define MAX_TEXTURES 4

struct VertexData {
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

class MeshData : public IData {
public:
    Lex faces;
    std::vector<VertexData> vertices;
    std::vector<int> indices;
    unsigned int VAO, VBO, EBO;

    glm::mat4 localTransform;

    glm::mat4 getMeshMatrix() {
        return localTransform;
    }
    void execute() override {
        // cook this data so it is stable for remaining passes
    }
    std::string getName() override { return name_; } 

    MeshData() : VAO(0), VBO(0), EBO(0) {}
    MeshData(std::string name, Lex data) : name_(name), VAO(0), VBO(0), EBO(0), data_(data) {

        Log::console("Creating mesh_data: " + name, 1);
    }
    ~MeshData() {
        std::string logMessage = "Destroying Mesh with VAO: " + std::to_string(VAO);
        Log::console(logMessage, 1);
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

    void load() {
        VoxaData voxa(name_, data_);
        voxa.unpackIdArray();

        Lex dataN = voxa.bakeToMesh(true);

        //Log::console(dataN.getString());
        
        data_ = dataN;

        setupFromFaces();

        Log::console("Vertices initialized: " + std::to_string(vertices.size()), 1);
        Log::console("Indices initialized: " + std::to_string(indices.size()), 1); 

        setupMesh();
    }

    void render() {
        // std::cout << "Rendering Mesh with VAO: " << VAO << std::endl;
        // draw mesh

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void setupMesh() {
        Log::console("Setting up Mesh... ", 1);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        std::string logMessage = "Setting up Mesh with VAO: " + std::to_string(VAO);
        Log::console(logMessage, 1);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

        // VertexData Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
        // VertexData Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
        // VertexData TextureDataLite Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));
        // VertexData Colors
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));
        // ids
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(VertexData), (void*)offsetof(VertexData, m_BoneIDs));
		// weights
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, m_Weights));

        glBindVertexArray(0); // Unbind VAO
    }

private:
    std::string name_;
    Lex data_;

    void setupFromFaces() {
        faces = data_.getC<Lex>("faces", Lex());

        int vertexCount = getVertexCountFromFaces();
        int faceCount = faces.length();

        vertices.reserve(vertexCount);
        indices.reserve(faceCount * 3 * 2);

        for (const auto& kvp : faces) {
            Lex face = std::any_cast<Lex>(kvp.second);
            glm::vec3 normal = face.getVec("normal", glm::vec3(0.0f));
            //Log::console("normal: " + std::to_string(normal.x) + ", " + std::to_string(normal.y) + ", " + std::to_string(normal.z));
            triangulateFace(face);
        }
    }
    int getVertexCountFromFaces() {
        int count = 0;
        for (const auto& kvp : faces) {
            Lex face = std::any_cast<Lex>(kvp.second);
            Lex vertices = face.getC<Lex>("vertices", Lex());
            count += vertices.length();
        }
        return count;
    }
    void triangulateFace(const Lex& face) {

        Lex vertexLex = face.getC<Lex>("vertices", Lex());
        int startIndex = vertices.size();
        for (int i = 0; i < vertexLex.length(); ++i) {
            Lex vertexData = vertexLex.getC<Lex>(std::to_string(i), Lex());
            VertexData vertex;
            vertex.position = vertexData.getVec("position", glm::vec3(0.0f));
            vertex.normal = vertexData.getVec("normal", glm::vec3(0.0f));
            //Log::console("normal: " + std::to_string(vertex.normal.x) + ", " + std::to_string(vertex.normal.y) + ", " + std::to_string(vertex.normal.z));
            vertex.uv = vertexData.getVec2("uv", glm::vec2(0.0f));
            vertex.color = vertexData.getVec("color", glm::vec3(1.0f));
            auto boneIDs = vertexData.getC<std::vector<int>>("bone_ids", std::vector<int>(MAX_BONE_INFLUENCE, 0));
            auto weights = vertexData.getC<std::vector<float>>("weights", std::vector<float>(MAX_BONE_INFLUENCE, 0.0f));
            std::copy(boneIDs.begin(), boneIDs.end(), vertex.m_BoneIDs);
            std::copy(weights.begin(), weights.end(), vertex.m_Weights);
            vertices.push_back(vertex);
        }

        // add two triangles
        indices.push_back(startIndex);
        indices.push_back(startIndex + 1);
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex);
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 3);
    }

}; // Added missing semicolon here at the end of the class definition


#endif // MESH_H
