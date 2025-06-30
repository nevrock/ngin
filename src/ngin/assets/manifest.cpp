
#include <ngin/assets/bucket.h>  
#include <ngin/atlas/atlas.h>

#include <ngin/assets/mesh.h>
#include <ngin/assets/shader.h>

#include <ngin/debug/logger.h>

// Function to create an instance of MyCustomAsset
Asset* create_atlas(const std::string& name) {
    return new Atlas(name);
}
Asset* create_mesh(const std::string& name) {
    return new MeshAsset(name);
}
Asset* create_shader(const std::string& name) {
    return new ShaderAsset(name);
}



void register_all_assets() {
    Logger logger("AssetManifest");
    logger.info("Registering assets");
    // atlas
    AssetsBucket::register_asset("atlas", create_atlas);
    logger.info("Atlas registered", 1);
    // mesh
    AssetsBucket::register_asset("mesh", create_mesh);
    logger.info("Mesh registered", 1);
    // shader
    AssetsBucket::register_asset("shader", create_shader);
    logger.info("Shader registered", 1);
}
bool register_assets = []() {
    register_all_assets();
    return true;
}();
 