#ifndef TRANSFORM_MODULE_H
#define TRANSFORM_MODULE_H

#include <string>
#include <vector>
#include <algorithm>

#include <ngin/scene/module/module.h>

namespace ngin {
namespace scene {

class TransformModule : public Module {
public:
    TransformModule (const unsigned int id, const std::string& name) : Module(id, name) {} // Initialize parent_ here

private:
};

}
}

#endif // TRANSFORM_MODULE_H