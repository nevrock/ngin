#ifndef TEXTURE_DATA_H
#define TEXTURE_DATA_H

#include <glad/glad.h>

#include <ngin/data/i_data.h>

// TextureData is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class TextureData : public IData 
{
public:

    TextureData() {
        glGenTextures(1, &this->id);
    }

    void execute() override {
        // cook this data so it is stable for remaining passes
    }

    std::string getName() override { return name_; } 

    // holds the id of the texture object, used for all texture operations to reference to this particular texture
    unsigned int id;
    // texture image dimensions
    unsigned int width, height; // width and height of loaded image in pixels
    // texture Format
    unsigned int internalFormat; // format of texture object
    unsigned int imageFormat; // format of loaded image
    // texture configuration
    unsigned int wrapS; // wrapping mode on S axis
    unsigned int wrapT; // wrapping mode on T axis
    unsigned int filterMin; // filtering mode if texture pixels < screen pixels
    unsigned int filterMax; // filtering mode if texture pixels > screen pixels

    // generates texture from image data
    void generate(unsigned int width, unsigned int height, unsigned char* data) {
        this->width = width;
        this->height = height;
        // create Texture
        glBindTexture(GL_TEXTURE_2D, this->id);
        glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, width, height, 0, this->imageFormat, GL_UNSIGNED_BYTE, data);
        
        glGenerateMipmap(GL_TEXTURE_2D);

        // set Texture wrap and filter modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax);
        // unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // binds the texture as the current active GL_TEXTURE_2D texture object
    void bind() const {
        glBindTexture(GL_TEXTURE_2D, this->id);
    }

private:
    std::string name_;
};

#endif