#ifndef MODULE_DATA_H
#define MODULE_DATA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

class ModuleData {    

public:
    ModuleData(std::string name) : name_(name) {}
    ~ModuleData() {
        delete args_;
    }

    void from_atlas(Atlas* data) {
        std::string* kind_ptr = nullptr;
        kind_ptr = data->get<std::string>("kind", kind_ptr);
        if (kind_ptr) {
            kind_ = *kind_ptr;
        }

        args_ = data;
    }

    Atlas* get_args() {
        return args_;
    }
    std::string& get_name() {
        return name_;
    }
    std::string& get_kind() {
        return kind_;
    }




    size_t get_deep_memory_usage() const {
        size_t total_memory = sizeof(*this); // Size of the ModuleData object itself

        // Add memory for std::string internal buffers
        total_memory += name_.capacity();
        total_memory += kind_.capacity();

        // Add memory for the Atlas pointed to by args_
        // This is crucial because ModuleData owns and deletes args_.
        // Assuming Atlas has a get_deep_memory_usage() or a way to determine its size.
        // For simplicity, let's estimate Atlas size. A real Atlas would likely
        // be complex and need its own deep memory calculation.
        if (args_) {
            // Placeholder: A real Atlas would need its own deep memory calculation.
            // Atlas is likely holding std::map<std::string, void*>, so it has:
            // 1. Size of the map object itself.
            // 2. Memory for map nodes (key strings, void* pointers).
            // 3. Memory for the actual data pointed to by void* (e.g., std::string, std::vector, other Atlases).
            // This is a rough estimation. For precision, Atlas needs its own get_deep_memory_usage.
            total_memory += sizeof(Atlas); // Base size of the Atlas object
            // You would then add memory for internal map, keys, and values.
            // E.g., if Atlas stores std::string keys and variant values, it gets complex.
            // For this example, let's assume a basic Atlas size for now.
            // If Atlas is recursive (e.g., `data->get<Atlas>("child_atlas")`),
            // its `get_deep_memory_usage()` would also need to be recursive.
            // For now, we'll just add a placeholder.
            // A more realistic Atlas memory calculation would need access to its internals.
        }

        return total_memory;
    }

private:
    std::string name_;
    std::string kind_;

    Atlas* args_;
};

#endif // MODULE_DATA_H