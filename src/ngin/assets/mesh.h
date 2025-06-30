#ifndef MESH_ASSET_H
#define MESH_ASSET_H

#include <ngin/assets/asset.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>
#include <ngin/render/gl/mesh/data.h>

class MeshAsset : public Asset {
public:    
    MeshAsset(std::string name) : Asset(name), data_(), gl_data_(data_) {
        logger_ = new Logger("MeshAsset::" + name);
    }
    ~MeshAsset() {
        delete logger_;
    }

    void read(const std::string& filepath) override {
        Atlas* data = new Atlas();
        data->read(filepath);
        data_.from_data(*data, logger_);

        logger_->info("Mesh asset loaded from: " + filepath);
    }
    void write(const std::string& filepath) const override {
    }
    
private:
    Logger* logger_;
    
    MeshData data_;
    GlMeshData gl_data_;
};

#endif // MESH_ASSET_H
