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
#include <ngin/data/mesh/mesh_converter_atl.h>
#include <ngin/data/mesh/mesh_converter_obj.h>

class MeshAsset : public Asset {
public:    
    MeshAsset(unsigned int id, std::string name) : Asset(id, name), data_(), gl_data_(data_) {
        logger_ = new ngin::debug::Logger("MeshAsset::" + name);
    }
    ~MeshAsset() {
        logger_->info("MeshAsset cleanup --> " + get_name());
        delete logger_;
    }

    void read(const std::string& filepath, ngin::debug::Printer& debug) override {
        if (filepath.substr(filepath.find_last_of(".") + 1) == "nmesh") {
            Atlas* data = new Atlas();
            data->read(filepath);

            MeshConverterAtlas converter;
            converter.from_data(*data, data_, logger_);
            delete data;
        } else if (filepath.substr(filepath.find_last_of(".") + 1) == "obj") { // Assume .obj for now
            MeshConverterObj converter;
            converter.from_file(filepath, data_, logger_);
        } else {
            logger_->info("Unsupported mesh file format: " + filepath);
            return;
        }
    }
    void write(const std::string& filepath) const override {
    }
    void refresh_gl_data() override {
        logger_->info("Refreshing GL data for mesh: " + get_name());
        gl_data_.refresh();
    }
private:
    ngin::debug::Logger* logger_;
    
    MeshData data_;
    GlMeshData gl_data_;
};

#endif // MESH_ASSET_H
