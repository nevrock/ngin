#ifndef TRANSFORM_MODULE_H
#define TRANSFORM_MODULE_H

#include <string>
#include <vector>
#include <algorithm>

#include <ngin/scene/module/module.h>
#include <ngin/data/transform.h>

namespace ngin {
namespace scene {

class TransformModule : public Module {
public:
    TransformModule (const unsigned int id, const std::string& name) : Module(id, name) {

    } // Initialize parent_ here

    void from_atlas(Atlas* data) override {
        data_.from_atlas(data);
    }

private:
    TransformData data_;
};
}
}

#endif // TRANSFORM_MODULE_H