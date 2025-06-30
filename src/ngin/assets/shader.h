#ifndef SHADER_ASSET_H
#define SHADER_ASSET_H

#include <ngin/assets/asset.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>
#include <ngin/render/gl/shader/data.h>

class ShaderAsset : public Asset
{
public:
    ShaderAsset(std::string name) : Asset(name), data_(), gl_data_(name, data_) {
        logger_ = new Logger("ShaderAsset::" + name);
    }
    ~ShaderAsset() {
        delete logger_;
    }

    void read(const std::string &filepath) override {
        Atlas *data = new Atlas();
        data->read(filepath);
        data_.from_data(*data, logger_);
        gl_data_.load();

        logger_->info("Shader asset loaded from: " + filepath);
    }
    void write(const std::string &filepath) const override
    {
    }

private:
    Logger *logger_;

    ShaderData data_;
    GlShaderData gl_data_;
};

#endif // SHADER_ASSET_H
