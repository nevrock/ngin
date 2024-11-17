#ifndef MODEL_ANIMATION_H
#define MODEL_ANIMATION_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <nev/model.h>
#include <nev/animator.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <nev/assimp_glm_helpers.h>

using namespace std;

class ModelAnimation : public Model
{
public:
	ModelAnimation(IObject* parent) : Model(parent) {}

    void launch() override {
        Model::launch();
        // Assuming 'parent' is a pointer to an IObject or derived class, cast it to Object*
        Object* o = dynamic_cast<Object*>(parent);
        if (o) {  // Check if the cast is successful
            animator = o->getComponent<Animator>();
        } else {
            // Handle the case where parent is not an Object or the cast fails
            std::cerr << "Parent cast to Object* failed" << std::endl;
        }
    }
    void updateBonesCheck(Shader& shader) {
        shader.setBool("IS_ANIMATION", false);
        if (animator == nullptr || !isAnim) return;

        std::vector<glm::mat4> boneMatrices = animator->getFinalBoneMatrices();
        for (unsigned int i = 0; i < boneMatrices.size(); ++i)
            shader.setMat4("A_M_BONES[" + std::to_string(i) + "]", boneMatrices[i]);

        shader.setBool("IS_ANIMATION", true);
        shader.setInt("NUM_BONES", 4);
    }

	auto& getBoneInfoMap() { return data->getBoneInfoMap(); }
	int& getBoneCount() { return data->getBoneCount(); }
	
protected:
    bool isAnim = false;
    std::shared_ptr<Animator> animator;
};



#endif
