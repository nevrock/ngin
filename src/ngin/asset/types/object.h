#ifndef OBJECT_ASSET_H
#define OBJECT_ASSET_H

#include <vector>
#include <string>
#include <map>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

#include <ngin/data/object.h>

class ObjectAsset : public Asset {
public:    
    ObjectAsset(unsigned int id, std::string name) : Asset(id, name) {
        logger_ = new ngin::debug::Logger("ObjectAsset::" + name);
    }
    ~ObjectAsset() {
        if (data_) {
            delete data_;
        }
        delete logger_;
    }

    ObjectData* get_data() {
        return data_;
    }

    void read(const std::string& filepath, ngin::debug::Printer& debug) override {
        Atlas* data = new Atlas();
        data->read(filepath);        

        Atlas* children = data->get<Atlas>("children");
        data_ = new ObjectData();
        if (children) {
            for (auto& child_name : children->keys()) {
                Atlas* child_data = children->get<Atlas>(child_name);
                data_->from_atlas(child_data);
                break;
            }
        } else {
            data_->from_atlas(data);
        }
        debug.info("Loaded object asset: " + filepath + ", size: " + std::to_string(data_->get_deep_memory_usage()) + " bytes", debug_name_);
    }
    void write(const std::string& filepath) const override {

    }
    
private:
    ngin::debug::Logger* logger_;
    ObjectData* data_;

    std::string debug_name_ = "ObjectAsset::";
    
};

#endif // OBJECT_ASSET_H
