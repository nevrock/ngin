#ifndef VOXA_DATA_H
#define VOXA_DATA_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <array>
#include <map>

#include <ngin/data/i_data.h>   
#include <ngin/lex.h>
#include <ngin/log.h>

class VoxaData : public IData {
public:

    void execute() override {
        // cook this data so it is stable for remaining passes
    }

    std::string getName() override { return name_; } 

    VoxaData(std::string name, Lex data) : name_(name), data_(data) {
        Log::console("Creating voxa_data: " + name, 1);
    }

    ~VoxaData() {
        
    }

    void unpackIdArray() {
        auto shape = data_.getC<std::vector<int>>("shape", {});
        if (shape.size() != 4) {
            Log::console("Invalid shape size", 1);
            return;
        }

        int x = shape[0], y = shape[1], z = shape[2], w = shape[3];
        idArray_.resize(x, std::vector<std::vector<std::vector<int>>>(y, std::vector<std::vector<int>>(z, std::vector<int>(w, 0))));

        auto flatArray = data_.getC<std::vector<int>>("id_array", {});
        if (flatArray.size() != x * y * z * w) {
            Log::console("Invalid id_array size", 1);
            return;
        }

        int index = 0;
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                for (int k = 0; k < z; ++k) {
                    for (int l = 0; l < w; ++l) {
                        idArray_[i][j][k][l] = flatArray[index++];
                    }
                }
            }
        }
    }

    // Helper function to get an id value from idArray
    int getId(int x, int y, int z, int w) const {
        if (x < 0 || y < 0 || z < 0 || w < 0 || 
            x >= idArray_.size() || y >= idArray_[0].size() || 
            z >= idArray_[0][0].size() || w >= idArray_[0][0][0].size()) {
            return 0; // Return 0 for out-of-bounds access
        }
        return idArray_[x][y][z][w];
    }

    // Helper function to set an id value in idArray
    void setId(int x, int y, int z, int w, int value) {
        if (x >= 0 && y >= 0 && z >= 0 && w >= 0 && 
            x < idArray_.size() && y < idArray_[0].size() && 
            z < idArray_[0][0].size() && w < idArray_[0][0][0].size()) {
            idArray_[x][y][z][w] = value;
        }
    }

    Lex bakeToMesh(bool renderBottomFaces = false, int padding = 0) {
        Lex mesh;
        mesh.set("name", data_.getC<std::string>("magicavoxel_file", "voxel_mesh"));
        Lex faces;

        auto uniqueColors = data_.getC<Lex>("unique_colors", {});
        int faceIndex = 0;

        auto isVisible = [&](int x, int y, int z) {
            return x < 0 || y < 0 || z < 0 || x >= idArray_.size() || y >= idArray_[0].size() || z >= idArray_[0][0].size() || idArray_[x][y][z][0] == 0;
        };
        int sizeX = idArray_.size(); 
        sizeX -= padding;

        int sizeY = idArray_[0].size();
        sizeY -= padding;

        int sizeZ = idArray_[0][0].size();
        sizeZ -= padding;

        for (int x = padding; x < sizeX; ++x) {
            for (int y = padding; y < sizeY; ++y) {
                for (int z = padding; z < sizeZ; ++z) {
                    int id = idArray_[x][y][z][0];
                    if (id == 0) continue;

                    auto colorData = uniqueColors.getC<Lex>(std::to_string(id), {});
                    auto uv = colorData.getVec2("uv", glm::vec2(0.0f));

                    auto addFace = [&](std::vector<glm::vec3> positions, glm::vec3 normal) {
                        Lex face;
                        Lex vertices;
                        for (int i = 0; i < positions.size(); ++i) {
                            Lex vertex;

                            std::vector<float> positionV = {positions[i].x, positions[i].y, positions[i].z};
                            std::vector<float> normalV = {normal.x, normal.y, normal.z};
                            std::vector<float> uvV = {uv.x, uv.y};

                            vertex.set("position", positionV);
                            vertex.set("normal", normalV);
                            vertex.set("uv", uvV);

                            vertices.set(std::to_string(i), vertex);
                        }

                        face.set("vertices", vertices);
                        std::vector<float> normalFace = {normal.x, normal.y, normal.z};
                        face.set("normal", normalFace);

                        faces.set(std::to_string(faceIndex++), face);
                    };

                    if (isVisible(x - 1, y, z)) addFace({{x, y, z}, {x, y, z + 1}, {x, y + 1, z + 1}, {x, y + 1, z}}, {-1, 0, 0});
                    if (isVisible(x + 1, y, z)) addFace({{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y + 1, z + 1}, {x + 1, y, z + 1}}, {1, 0, 0});
                    if (isVisible(x, y - 1, z)) addFace({{x, y, z}, {x + 1, y, z}, {x + 1, y, z + 1}, {x, y, z + 1}}, {0, -1, 0});
                    if (isVisible(x, y + 1, z)) addFace({{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z + 1}, {x + 1, y + 1, z}}, {0, 1, 0});
                    if (isVisible(x, y, z - 1)) addFace({{x, y, z}, {x, y + 1, z}, {x + 1, y + 1, z}, {x + 1, y, z}}, {0, 0, -1});
                    if (renderBottomFaces && isVisible(x, y, z + 1)) addFace({{x, y, z + 1}, {x + 1, y, z + 1}, {x + 1, y + 1, z + 1}, {x, y + 1, z + 1}}, {0, 0, 1});
                }
            }
        }

        mesh.set("faces", faces);

        return mesh;
    }

    Lex bakeToMeshGreedy(bool renderBottomFaces = false, int padding = 0) {
        Lex mesh = bakeToMesh(renderBottomFaces, padding);
        Lex faces = mesh.getC<Lex>("faces", {});
        Lex mergedFaces;

        auto areFacesMergeable = [](const Lex& face1, const Lex& face2) {
            auto normal1 = face1.getC<std::vector<float>>("normal", {});
            auto normal2 = face2.getC<std::vector<float>>("normal", {});
            if (normal1 != normal2) return false;

            auto vertices1 = face1.getC<Lex>("vertices", {});
            auto vertices2 = face2.getC<Lex>("vertices", {});

            std::vector<std::vector<float>> positions1, positions2;
            for (int i = 0; i < 4; ++i) {
                positions1.push_back(vertices1.getC<Lex>(std::to_string(i), {}).getC<std::vector<float>>("position", {}));
                positions2.push_back(vertices2.getC<Lex>(std::to_string(i), {}).getC<std::vector<float>>("position", {}));
            }

            // Check if the faces share an edge and are aligned
            int sharedVertices = 0;
            for (const auto& pos1 : positions1) {
                for (const auto& pos2 : positions2) {
                    if (pos1 == pos2) {
                        ++sharedVertices;
                    }
                }
            }
            return sharedVertices == 2; // Mergeable if they share exactly 2 vertices
        };

        auto mergeFaces = [](const Lex& face1, const Lex& face2) {
            Lex mergedFace;
            auto normal = face1.getC<std::vector<float>>("normal", {});
            mergedFace.set("normal", normal);

            auto vertices1 = face1.getC<Lex>("vertices", {});
            auto vertices2 = face2.getC<Lex>("vertices", {});

            std::vector<std::vector<float>> positions1, positions2;
            for (int i = 0; i < 4; ++i) {
                positions1.push_back(vertices1.getC<Lex>(std::to_string(i), {}).getC<std::vector<float>>("position", {}));
                positions2.push_back(vertices2.getC<Lex>(std::to_string(i), {}).getC<std::vector<float>>("position", {}));
            }

            // Combine unique positions
            std::vector<std::vector<float>> mergedPositions = positions1;
            for (const auto& pos : positions2) {
                if (std::find(mergedPositions.begin(), mergedPositions.end(), pos) == mergedPositions.end()) {
                    mergedPositions.push_back(pos);
                }
            }

            // Sort positions to maintain order
            std::sort(mergedPositions.begin(), mergedPositions.end());

            Lex mergedVertices;
            for (int i = 0; i < mergedPositions.size(); ++i) {
                Lex vertex;
                vertex.set("position", mergedPositions[i]);
                vertex.set("normal", normal);
                mergedVertices.set(std::to_string(i), vertex);
            }

            mergedFace.set("vertices", mergedVertices);
            return mergedFace;
        };

        std::map<int, bool> visited;
        int mergedFaceIndex = 0;

        for (const auto& [key1, face1Any] : faces.getMap()) {
            Lex face1 = std::any_cast<Lex>(face1Any); // Properly cast std::any to Lex
            if (visited[std::stoi(key1)]) continue;

            Lex currentFace = face1;
            visited[std::stoi(key1)] = true;

            for (const auto& [key2, face2Any] : faces.getMap()) {
                Lex face2 = std::any_cast<Lex>(face2Any); // Properly cast std::any to Lex
                if (visited[std::stoi(key2)]) continue;

                if (areFacesMergeable(currentFace, face2)) {
                    currentFace = mergeFaces(currentFace, face2);
                    visited[std::stoi(key2)] = true;
                }
            }

            mergedFaces.set(std::to_string(mergedFaceIndex++), currentFace);
        }

        mesh.set("faces", mergedFaces);
        return mesh;
    }

private:
    std::string name_;
    Lex data_;
    std::vector<std::vector<std::vector<std::vector<int>>>> idArray_;
}; 

#endif // VOXA_DATA_H
