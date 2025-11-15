#ifndef GL_TEXTURE_DATA_H
#define GL_TEXTURE_DATA_H

#include <fstream> // Add this include directive
#include <sstream> // Add this include directive
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ngin/debug/printer.h>
#include <ngin/data/texture.h>

class GlTextureData {
public:
    GlTextureData(const std::string name, TextureData& data) : name_(name), data_(data), logger_{"GlTextureData::" + name_} {
    }

    void refresh() {
        GLenum format;
        if (data_.n_channels == 1)
            format = GL_RED;
        else if (data_.n_channels == 3)
            format = GL_RGB;
        else if (data_.n_channels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, data_.id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, data_.width, data_.height, 0, format, GL_UNSIGNED_BYTE, data_.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        logger_.info("Texture data refreshed for: " + name_ + ", with id: " + std::to_string(data_.id));

        data_.clear_data(); // Clear the data after uploading to OpenGL
    }


private:
    ngin::debug::Logger logger_;
    const std::string name_;

    TextureData& data_;
};

#endif