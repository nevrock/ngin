#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <nev/bone.h>
#include <nev/object.h>
#include <nev/resources.h>

class Animator : public Component
{
public:
	Animator(Object* parent);
	
	void loadFromDict(const Dict& d) override {
		std::string fileName = d.getC<std::string>("file", "");
		m_FileName = fileName;
		m_IsValid = false;
		//setup(Resources::getAnimation(fileName));
	}
	void launch() override {
		// now we can load
		setup(Resources::getAnimation(m_FileName));
		m_IsValid = true;
    }
	void setup(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;
		m_FinalBoneMatrices.reserve(100);
		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}


	void updateAnimation(float dt) override
	{
		if (!m_IsValid) return;

		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
			calculateBoneTransforms(&m_CurrentAnimation->getRootNode(), glm::mat4(1.0f));
		}
	}

	void playAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}
	void calculateBoneTransforms(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		//Log::console("Visiting node: " + nodeName); // Log the current node being processed

		Bone* bone = m_CurrentAnimation->findBone(nodeName);

		if (bone)
		{
			//Log::console("Found bone: " + nodeName + ", updating animation.");
			bone->updateAnimation(m_CurrentTime);
			nodeTransform = bone->getLocalTransform();
			//Log::console("Updated node transform for bone: " + nodeName);
		}
		else
		{
			//Log::console("No bone found for node: " + nodeName + ", using default node transform.");
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		//Log::console("Computed global transformation for node: " + nodeName);

		auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;

			//Log::console("Bone info updated for: " + nodeName + " at index " + std::to_string(index));
		}
		else
		{
			//Log::console("No bone info found for node: " + nodeName);
		}

		for (int i = 0; i < node->childrenCount; i++)
		{
			//Log::console("Recursively calculating transforms for child " + std::to_string(i) + " of node: " + nodeName);
			calculateBoneTransforms(&node->children[i], globalTransformation);
		}
	}
	std::vector<glm::mat4> getFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

	std::string m_FileName;
	bool m_IsValid;
};