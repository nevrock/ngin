#ifndef TEXTURE_ASSET_H
#define TEXTURE_ASSET_H

#include <ngin/asset/asset.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

#include <ngin/render/gl/texture/data.h>

class TextureAsset : public Asset
{
public:
    TextureAsset(unsigned int id, std::string name) : Asset(id, name), texture_data_(), gl_texture_data_(name, texture_data_) {
        logger_ = new ngin::debug::Logger("TextureAsset::" + name);
    }
    ~TextureAsset() {
        delete logger_;
    }

    void read(const std::string &filepath, ngin::debug::Printer &debug) override {
        debug.info("Reading texture from file: " + filepath, "TextureAsset");

        texture_data_.from_data(filepath, debug);
    }
    void write(const std::string &filepath) const override
    {
    }
    void refresh_gl_data() override {
        logger_->info("Refreshing GL data for texture: " + get_name());
        gl_texture_data_.refresh();
    }
private:
    ngin::debug::Logger *logger_;
    TextureData texture_data_; // Holds the texture data
    GlTextureData gl_texture_data_; // OpenGL texture data
};

#endif // TEXTURE_ASSET_H
