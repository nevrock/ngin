#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <nev/dict.h>  // Include the JSON library header

#include <nev/log.h>

struct ModelData {
public:
    std::string name;
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<Texture> textures;
    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;

    Mesh* getMesh(int index) {
        if (index < meshes.size()) {
            return meshes[index].get();
        } return nullptr;
    }

    auto& getBoneInfoMap() { return boneInfoMap; }
	int& getBoneCount() { return boneCounter; }

    void render() {
        for (auto& mesh : meshes) {
            mesh->render();
        }
    }
    void setName(std::string name) { 
        this->name = name;
    }
};

class ModelLoader {
public:

    static Dict loadDict(const std::string& name) {
        return loadDictFile(FileUtils::getResourcePath("nev/" + name).c_str());
    }
    static Dict loadDictFile(const char* filePath) {
        Dict d;
        d.read(filePath);
        return d;
    }

    static void loadAssimpModel(const aiScene* scene, ModelData& data) {
        // process ASSIMP's root node recursively
        if (scene == nullptr) {
            Log::console("model fbx has null scene");
        }
        processNode(scene->mRootNode, scene, data);
    }
    static void processNode(aiNode *node, const aiScene *scene, ModelData& data)
    {   
        if (node == nullptr) {
            Log::console("model fbx has null node");
            return;
        }
        Log::console("processing node: " + std::string(node->mName.data) + ", mesh count: " + std::to_string(node->mNumMeshes));

        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene, data);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, data);
        }

    }
    static void setVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }
    static void processMesh(aiMesh* mesh, const aiScene* scene, ModelData& data)
    {
        //Log::console("resources process mesh has " + std::to_string(mesh->mNumAnimMeshes) + " blend shapes.");

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Dict j = loadDict("models/" + data.name + ".nevf");
        j.print();
        int texIndex = 2;
        for (const auto& textureName : j.getC<std::vector<std::string>>("textures", std::vector<std::string>{""})) {
            if (textureName == "")
                continue;

            Log::console("resources loading texture for mesh! " + textureName);
            Texture texture;
            texture.id = texIndex,
            texture.name = textureName;
            textures.push_back(texture);

            texIndex += 1;
        }

        // Prepare to log blend shapes names
        std::string blendShapeNames;
        for (unsigned int i = 0; i < mesh->mNumAnimMeshes; i++) {
            if (i > 0) blendShapeNames += ", ";
            blendShapeNames += mesh->mAnimMeshes[i]->mName.data;
        }

        // Log the mesh details including the number of blend shapes and their names
        Log::console("processing mesh: " + std::string(mesh->mName.data) + 
                    ", vertices count: " + std::to_string(mesh->mNumVertices) + 
                    ", faces count: " + std::to_string(mesh->mNumFaces) +
                    ", blend shapes count: " + std::to_string(mesh->mNumAnimMeshes) +
                    ", blend shapes names: [" + blendShapeNames + "]");


        //Log::console("processing mesh: " + std::string(mesh->mName.data) + ", vertices count: " + std::to_string(mesh->mNumVertices) + ", faces count: " + std::to_string(mesh->mNumFaces));

        vertices.reserve(mesh->mNumVertices);
        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            setVertexBoneDataToDefault(vertex);
            vertex.position = AssimpGLMHelpers::getGLMVec(mesh->mVertices[i]);
            vertex.normal = AssimpGLMHelpers::getGLMVec(mesh->mNormals[i]);
            
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }
            else
            {
                vertex.uv = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
        
        unsigned int totalIndices = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            totalIndices += mesh->mFaces[i].mNumIndices;
        }
        indices.reserve(totalIndices);
        Log::console("resources process mesh has " + std::to_string(mesh->mNumAnimMeshes) + " blend shapes.");

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        Log::console("completed transfer of vertices");
        // Process bones
        extractBoneWeightForVertices(vertices, mesh, scene, data);

        Log::console("completed bone weight extraction");

        auto newMesh = std::make_unique<Mesh>();
        newMesh->vertices = std::move(vertices);
        newMesh->indices = std::move(indices);
        newMesh->textures = std::move(textures);
        newMesh->setupMesh();  

        // Process each animMesh as a separate Mesh object and store it as a shape key
        for (unsigned int i = 0; i < mesh->mNumAnimMeshes; i++) {
            Mesh shapeKeyMesh;
            aiAnimMesh* animMesh = mesh->mAnimMeshes[i];

            // Assuming animMesh vertices replace the main mesh vertices directly
            for (unsigned int j = 0; j < animMesh->mNumVertices; j++) {
                Vertex vertex;
                // Populate vertex data from animMesh
                vertex.position = AssimpGLMHelpers::getGLMVec(animMesh->mVertices[j]);
                if (animMesh->HasNormals()) {
                    vertex.normal = AssimpGLMHelpers::getGLMVec(animMesh->mNormals[j]);
                }
                // UVs and other per-vertex data should be copied from the main mesh if they don't change
                vertex.uv = newMesh->vertices[j].uv;

                shapeKeyMesh.vertices.push_back(vertex);
            }
            shapeKeyMesh.indices = newMesh->indices;  // Use the same indices as the main mesh
            shapeKeyMesh.textures = newMesh->textures;  // Use the same textures as the main mesh

            // Setup the shape key mesh (VBO, VAO, etc.) if needed
            shapeKeyMesh.setupMesh();

            newMesh->addShapeKey(shapeKeyMesh);
        }




        data.meshes.push_back(std::move(newMesh));
    }
    static void setVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }
    static void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, ModelData& data)
    {
        auto& boneInfoMap = data.boneInfoMap;
        int& boneCount = data.boneCounter;

        Log::console("model fbx total bones in mesh: " + std::to_string(mesh->mNumBones));

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

            //Log::console("processing bone 01: " + boneName);

            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;  // Ensure boneCount is initialized correctly
                newBoneInfo.offset = AssimpGLMHelpers::convertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;  // Increment to prepare for the next new bone
            } else {
                boneID = boneInfoMap[boneName].id;
            }
            
            //Log::console("processing bone 02: " + boneName + ", ID: " + std::to_string(boneID));

            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            //Log::console("processing bone 03: " + boneName + ", ID: " + std::to_string(boneID) + ", Weights: " + std::to_string(numWeights));

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId < vertices.size());
                setVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }
    static std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        int textureCount = 2;
        std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            Log::console("model fbx load texture: " + std::string(str.C_Str()));
            
        }
        return textures;
    }
};

#endif // MODEL_LOADER_H
