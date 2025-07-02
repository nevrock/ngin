#ifndef MESH_ASSET_H
#define MESH_ASSET_H

#include <ngin/asset/asset.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>
#include <ngin/render/gl/mesh/data.h>

class MeshAsset : public Asset {
public:    
    MeshAsset(unsigned int id, std::string name) : Asset(id, name), data_(), gl_data_(data_) {
        logger_ = new ngin::debug::Logger("MeshAsset::" + name);
    }
    ~MeshAsset() {
        delete logger_;
    }

    void read(const std::string& filepath, ngin::debug::Printer& debug) override {
        Atlas* data = new Atlas();
        data->read(filepath);
        data_.from_data(*data);
    }
    void write(const std::string& filepath) const override {
    }
    
private:
    ngin::debug::Logger* logger_;
    
    MeshData data_;
    GlMeshData gl_data_;
};

#endif // MESH_ASSET_H
