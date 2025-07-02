#ifndef OBJECT_DATA_H
#define OBJECT_DATA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

#include <ngin/data/transform.h>
#include <ngin/data/module.h>

class ObjectData {    

public:
    ObjectData(ObjectData* parent = nullptr) : parent_(parent) {}
    ~ObjectData() {
        for (auto& child : children_) {
            delete child;
        }
        for (auto& module : modules_) {
            delete module;
        }
    }

    std::string& get_name() {
        return name_;
    }
    ObjectData* get_parent() {
        return parent_;
    }
    std::vector<ObjectData*>& get_children() {
        return children_;
    }
    std::vector<ModuleData*>& get_modules() {
        return modules_;
    }
    TransformData& get_transform() {
        return transform_;
    }


    void from_atlas(Atlas* data) {
        name_ = "";
        std::string* name_ptr = nullptr;
        name_ptr = data->get<std::string>("name", name_ptr);
        if (name_ptr) {
            name_ = *name_ptr;
        }

        Atlas* transform_atlas = nullptr;
        transform_atlas = data->get<Atlas>("transform", transform_atlas);
        if (transform_atlas) {
            transform_.from_atlas(transform_atlas);
        }
                                                                                                                                                                                                                                                                                                                                                                                                                                 
        Atlas* children_atlas = nullptr;
        children_atlas = data->get<Atlas>("children", children_atlas);
        if (children_atlas) {
            for (auto& child_name : children_atlas->keys()) {
                Atlas* child_data = children_atlas->get<Atlas>(child_name);
                ObjectData* child_obj = new ObjectData(this);
                children_.push_back(child_obj);
                child_obj->from_atlas(child_data);
            }
        }

        Atlas* modules_atlas = nullptr;
        modules_atlas = data->get<Atlas>("modules", modules_atlas);
        if (modules_atlas) {
            for (auto& module_name : modules_atlas->keys()) {
                Atlas* module_data = modules_atlas->get<Atlas>(module_name);
                ModuleData* module_obj = new ModuleData(module_name);
                modules_.push_back(module_obj);
                module_obj->from_atlas(module_data);
            }
        }
    }

    size_t get_deep_memory_usage() const {
        size_t total_memory = sizeof(*this); // Size of the ObjectData object itself

        // Add memory for std::string (its internal buffer)
        total_memory += name_.capacity(); 

        // parent_ is just a raw pointer, not owned memory, so its size is included in sizeof(*this)

        // Add memory for children_ vector's internal array of pointers
        total_memory += children_.capacity() * sizeof(ObjectData*);
        // Recursively add memory for each child ObjectData object
        for (const auto& child : children_) {
            if (child) {
                total_memory += child->get_deep_memory_usage();
            }
        }

        // Add memory for modules_ vector's internal array of pointers
        total_memory += modules_.capacity() * sizeof(ModuleData*);
        // Recursively add memory for each ModuleData object
        for (const auto& module : modules_) {
            if (module) {
                total_memory += module->get_deep_memory_usage();
            }
        }

        return total_memory;
    }

private:
    std::string name_;
    ObjectData* parent_;
    std::vector<ObjectData*> children_;
    std::vector<ModuleData*> modules_;
    TransformData transform_;
};

#endif // OBJECT_DATA_H