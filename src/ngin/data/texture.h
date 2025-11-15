#ifndef TEXTURE_DATA_H
#define TEXTURE_DATA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <stb_image.h>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

struct TextureData {
    unsigned int id; // OpenGL texture ID
    int width, height, n_channels; // Texture dimensions and number of channels

    unsigned char* data;

    TextureData() : id(0), width(0), height(0), n_channels(0), data(nullptr) {}
    ~TextureData() {
        if (data) {
            stbi_image_free(data);
            data = nullptr;
        }
    }
    void clear_data() {
        if (data) {
            stbi_image_free(data);
            data = nullptr;
        }
    }

    void from_data(const std::string& filepath, ngin::debug::Printer &debug) {
		data = stbi_load(filepath.c_str(), &width, &height, &n_channels, 0);

        debug.info("Loaded texture from file: " + filepath, "TextureData");
    }
};

#endif // TEXTURE_DATA_H