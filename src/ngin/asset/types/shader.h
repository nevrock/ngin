#ifndef SHADER_ASSET_H
#define SHADER_ASSET_H

#include <ngin/asset/asset.h>
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
    ShaderAsset(unsigned int id, std::string name) : Asset(id, name), data_(), gl_data_(name, data_) {
        logger_ = new ngin::debug::Logger("ShaderAsset::" + name);
    }
    ~ShaderAsset() {
        delete logger_;
    }

    void read(const std::string &filepath, ngin::debug::Printer &debug) override {
        Atlas *data = new Atlas();
        data->read(filepath);
        data_.from_data(*data);
        delete data;
    }
    void write(const std::string &filepath) const override
    {
    }
    void refresh_gl_data() override {
        logger_->info("Refreshing GL data for shader: " + get_name());
        gl_data_.refresh();
    }
private:
    ngin::debug::Logger *logger_;

    ShaderData data_;
    GlShaderData gl_data_;
};

#endif // SHADER_ASSET_H
