
#include <ngin/asset/bucket.h>  

#include <ngin/asset/types/mesh.h>
#include <ngin/asset/types/shader.h>
#include <ngin/asset/types/object.h>

#include <ngin/debug/logger.h>

std::shared_ptr<Asset> create_mesh(unsigned int id, const std::string& name) {
    return std::make_shared<MeshAsset>(id, name);
}
std::shared_ptr<Asset> create_shader(unsigned int id, const std::string& name) {
    return std::make_shared<ShaderAsset>(id, name);
}
std::shared_ptr<Asset> create_object(unsigned int id, const std::string& name) {
    return std::make_shared<ObjectAsset>(id, name);
}

void register_all_assets() {
    ngin::debug::Logger logger("AssetManifest");
    logger.info("Registering assets");

    // mesh
    ngin::asset::AssetBucket::register_asset("mesh", create_mesh);
    logger.info("Mesh registered", "", 1);
    // shader
    ngin::asset::AssetBucket::register_asset("shader", create_shader);
    logger.info("Shader registered", "", 1);
    // object
    ngin::asset::AssetBucket::register_asset("object", create_object);
    logger.info("Object registered", "", 1);
}

bool register_assets = []() {
    register_all_assets();
    return true;
}();
 