#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nev/bone.h>
#include <functional>
#include <nev/animdata.h>
#include <nev/log.h>

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class ModelAnimation;

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, ModelAnimation* model)
	{
		Assimp::Importer importer;
		unsigned int processFlags = 
			aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
			aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
			aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
			aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space (by default right-handed, for OpenGL)
			aiProcess_SortByPType | // ? 
			aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
			aiProcess_RemoveRedundantMaterials | // remove redundant materials
			aiProcess_FindDegenerates | // remove degenerated polygons from the import
			aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
			aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
			aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
			aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
			aiProcess_OptimizeMeshes | // join small meshes, if possible;
			aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
			0;

		const aiScene* scene = importer.ReadFile(animationPath, processFlags);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		readHierarchyData(m_RootNode, scene->mRootNode);
		readMissingBones(animation, *model);

		Log::console("animation " + animationPath + ", complete, with bones: " + std::to_string(m_Bones.size()));
		for (const Bone& bone : m_Bones) {
			Log::console("animation bone name: " + bone.m_Name);
		}
	}
	~Animation()
	{
	}
	Bone* findBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.getBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	inline float getTicksPerSecond() { return m_TicksPerSecond; }
	inline float getDuration() { return m_Duration;}
	inline const AssimpNodeData& getRootNode() { return m_RootNode; }
	inline const std::map<std::string,BoneInfo>& getBoneIDMap() 
	{ 
		return m_BoneInfoMap;
	}

private:
	void readMissingBones(const aiAnimation* animation, ModelAnimation& model);
	void readHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::convertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			readHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};

