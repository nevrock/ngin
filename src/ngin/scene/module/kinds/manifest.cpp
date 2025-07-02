
#include <ngin/scene/module/bucket.h>  
#include <ngin/scene/module/module.h>

#include <ngin/scene/module/kinds/transform.h>  

#include <ngin/debug/logger.h>

std::shared_ptr<ngin::scene::Module> create_transform(unsigned int id, const std::string& name) {
    return std::make_shared<ngin::scene::TransformModule>(id, name);
}

void register_all_modules() {
    ngin::debug::Logger logger("ModuleManifest");
    logger.info("Registering modules");

    // mesh
    ngin::scene::ModuleBucket::register_module("transform", create_transform);
    logger.info("Transform registered", "", 1);
}

bool register_modules = []() {
    register_all_modules();
    return true;
}();
 