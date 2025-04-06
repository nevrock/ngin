#ifndef ANIMATION_DATA_H
#define ANIMATION_DATA_H

#include <glad/glad.h>

#include <ngin/data/i_data.h>

// AnimationData is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class AnimationData : public IData 
{
public:

    AnimationData() {
    }

    void execute() override {
        // cook this data so it is stable for remaining passes
    }
    std::string getName() override { return name_; } 

private:
    std::string name_;

    
};

#endif