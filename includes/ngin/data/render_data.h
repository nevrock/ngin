#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include <ngin/data/i_data.h>
#include <ngin/data/shader_data.h>


class RenderData : public IData {
public:
    RenderData(std::string name, ShaderData& shader) : name_(name), shader_(shader) {

    }

    void execute() override {

    }

    std::string getName() override { return shader_.getName(); }

    ShaderData& getShader() { return shader_; }

private:
    ShaderData& shader_;
    std::string name_;
};

#endif // RENDER_DATA_H