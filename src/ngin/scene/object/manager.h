#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <parallel_hashmap/phmap.h>

#include <mutex>        // For std::mutex
#include <shared_mutex> // For std::shared_mutex (often preferred for read-heavy)

#include <memory>   // For std::shared_ptr, std::unique_ptr
#include <string>   // For std::string
#include <iostream> // For print statements (example usage)
#include <optional> // For std::optional (safe return values)
#include <utility>  // For std::move, std::forward

#include <ngin/scene/object/object.h>
#include <ngin/scene/object/context.h>
#include <ngin/job/collections/queue.h>
#include <ngin/scene/module/manager.h>
#include <ngin/job/collections/map.h> // Include the ParallelMap header

#include <ngin/asset/types/object.h>
#include <ngin/scene/module/kinds/transform.h>

#include <ngin/debug/bucket.h>
#include <ngin/debug/logger.h>

#include <ngin/util/id.h>

namespace ngin
{
namespace scene
{

class ObjectManager
{
public:

    ObjectManager() : module_mgr_(), logger_(ngin::debug::Logger("ObjectManager")) {
    }
    ~ObjectManager() {
    }

    void build_from_asset(ObjectAsset& object_asset) {
        // convert scene hierarchy into OMS (object-module-scene) - also known as eco (entity-component-scene)
        new_object(object_asset.get_data());
    }
    void new_object(ObjectData* data) {
        unsigned int id = IdUtil::get_unique_id();
        std::shared_ptr<Object> obj = std::make_shared<Object>(id, data->get_name());

        if (data->get_parent()) {
            auto parent = context_.get_object(data->get_parent()->get_name());
            if (parent) {
                obj->set_parent(parent->get_id());
                obj->set_level(parent->get_level() + 1);
            }
        } else {
            obj->set_parent(0);
            obj->set_level(0);
        }

        // TODO: modules go here
        unsigned int transform_id = module_mgr_.add_module(
            "transform",
            "transform",
            data->get_transform_atlas()  
        );
        TransformModule* transform_module = module_mgr_.get<TransformModule>("transform", "transform");
        if (transform_module) {
            transform_module->set_level(obj->get_level());
        }
        obj->add_module(transform_id);


        for (auto& module : data->get_modules()) {
            unsigned int module_id = module_mgr_.add_module(
                module->get_kind(), 
                module->get_name(),
                module->get_args()
            );
            if (module_id == 0) {
                logger_.info(
                    "Module not found: " + module->get_name()
                );
            }
            obj->add_module(module_id);
        }

        context_.add_object(id, obj);

        for (auto& child : data->get_children()) {
            new_object(child);
        }
    }
    std::vector<std::function<void()>> exeucte_object_edit_jobs() {
        std::vector<ObjectEdit> edits;
        edit_queue_.pop_all(edits);

        for (const auto& edit : edits) {
            execute_edit_(edit);
        }
        // This function signature returns std::vector<std::function<void()>>, but the implementation
        // does not currently return any. Depending on its intended use, it might need to be adjusted.
        return {}; 
    }

private:
    ngin::debug::Logger logger_;
    ngin::debug::DebugBucket debugger_;
    
    // Use ParallelMap directly
    ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Object>> object_map_;
    ngin::jobs::ParallelMap<std::string, unsigned int> name_to_id_map_;
    ngin::jobs::ParallelQueue<ObjectEdit> edit_queue_;

    ModuleManager module_mgr_;

    ObjectContext context_ = ObjectContext(object_map_, name_to_id_map_, edit_queue_, debugger_.get_context());

    void execute_edit_(const ObjectEdit& edit) {
        auto obj = context_.get_object(edit.object_id);
        if (!obj) {
            return;
        }
        switch (edit.kind) {
            case ObjectEditKind::EditParent: {
                obj->set_parent(edit.new_value);
                // readjust object hierarchy level too
                auto parent_obj = context_.get_object(edit.new_value);
                if (parent_obj) {
                    obj->set_level(parent_obj->get_level() + 1);
                }
                break;
            }
            case ObjectEditKind::AddChild: {
                obj->add_child(edit.new_value);
                // inform child too
                auto child_obj = context_.get_object(edit.new_value);
                if (child_obj) {
                    child_obj->set_parent(edit.object_id);
                    child_obj->set_level(obj->get_level() + 1);
                }
                break;
            }
            case ObjectEditKind::RemoveChild: {
                obj->remove_child(edit.new_value);
                // destroy child object
                context_.remove_object(edit.new_value);
                break;
            }
            case ObjectEditKind::AddModule: {
                obj->add_module(edit.new_value);
                break;
            }
            case ObjectEditKind::RemoveModule: {
                obj->remove_module(edit.new_value);
                break;
            }
        }
    }
};

}
}

#endif // OBJECT_MANAGER_H