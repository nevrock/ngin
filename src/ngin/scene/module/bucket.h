#ifndef MODULE_BUCKET_H
#define MODULE_BUCKET_H

#include <ngin/util/file.h>
#include <ngin/util/id.h>
#include <ngin/atlas/atlas.h>

#include <ngin/debug/logger.h>

#include <ngin/job/collections/vector.h>
#include <ngin/job/collections/map.h>
#include <ngin/scene/module/module.h>

#include <string>
#include <unordered_map>
#include <memory> // Include memory header for std::enable_shared_from_this
#include <functional>
#include <string>
#include <tuple>
#include <vector>

namespace ngin {
namespace scene {

class ModuleBucket {
public:

    ModuleBucket(const std::string& name) : name_(name) {
    }
    ~ModuleBucket() {
    }

    std::string& get_name() {
        return name_;
    }

    template<typename T>
    T* get(const std::string& name) {
        // Try to get by name_to_id_mapping_ first, which is more efficient
        std::optional<unsigned int> module_id_opt = name_to_id_mapping_.get(name);

        if (module_id_opt) {
            std::optional<std::shared_ptr<Module>> module_ptr_opt = modules_.get(module_id_opt.value());
            if (module_ptr_opt) {
                // Module found by ID
                T* module_asset = dynamic_cast<T*>(module_ptr_opt.value().get()); // Get raw pointer from shared_ptr
                if (module_asset) {
                    return module_asset;
                }
                // If dynamic_cast fails, it means the type T is not compatible.
                // Log an error or return nullptr as appropriate.
                logger_->error("Module '" + name + "' found but not of requested type.");
                return nullptr;
            }
        }
        logger_->warn("Module '" + name + "' not found or could not be loaded/casted.");
        return nullptr; // Module not found, or could not be casted, or not loaded
    }
    template<typename T>
    T* get(unsigned int id) {
        std::optional<std::shared_ptr<Module>> module_ptr_opt = modules_.get(module_id_opt.value());
        if (module_ptr_opt) {
            // Module found by ID
            
            T* module_asset = dynamic_cast<T*>(module_ptr_opt.value().get());
            if (module_asset) {
                return module_asset;
            }

            return nullptr;
        }
        return nullptr;
    }
    unsigned int get_module_count() {
        return modules_.size();
    }


    using ModuleFactory = std::function<std::shared_ptr<Module>(unsigned int id, const std::string& name)>;
    static ngin::jobs::ParallelMap<std::string, ModuleFactory>& get_module_factories() {
        static ngin::jobs::ParallelMap<std::string, ModuleBucket::ModuleFactory> module_factories_;
        return module_factories_;
    }
    static void register_module(const std::string& name, ModuleFactory factory) {
        get_module_factories().add(name, factory);
    }

private:
    ngin::debug::Logger* logger_ = new ngin::debug::Logger("ModuleBucket");
    std::string name_;

    ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Module>> modules_;
    ngin::jobs::ParallelMap<std::string, unsigned int> name_to_id_mapping_;

    std::string debug_name_ = "ModuleBucket::";
};

}
}

#endif // MODULE_BUCKET_H
