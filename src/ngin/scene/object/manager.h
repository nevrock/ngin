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

#include <ngin/debug/logger.h>

#include <ngin/util/id.h>

namespace ngin
{
namespace scene
{

class ObjectManager
{
public:

    ObjectManager() : module_mgr_() {
        logger_ = new ngin::debug::Logger("ObjectManager");
    }
    ~ObjectManager() {
        delete logger_;
    }

    void build_from_asset(ObjectAsset& object_asset) {
        // convert scene hierarchy into OMS (object-module-scene) - also known as eco (entity-component-scene)
        new_object(object_asset.get_data());
    }
    void new_object(ObjectData* data) {
        unsigned int id = IdUtil::get_unique_id();
        std::shared_ptr<Object> obj = std::make_shared<Object>(id, data->get_name());

        if (data->get_parent()) {
            auto parent = get_object(data->get_parent()->get_name());
            if (parent) {
                obj->set_parent(parent->get_id());
                obj->set_level(parent->get_level() + 1);
            }
        } else {
            obj->set_parent(0);
            obj->set_level(0);
        }

        // TODO: modules go here

        add_object(id, obj);

        for (auto& child : data->get_children()) {
            new_object(child);
        }
    }
    void clear_objects() {
        object_map_.clear();
        name_to_id_map_.clear();
    }

    void add_object(unsigned int id, std::shared_ptr<Object> obj)
    {
        // ParallelMap's add method is thread-safe
        object_map_.add(id, obj);
        name_to_id_map_.add(obj->get_name(), id);

        logger_->info("Added object " + obj->get_name() + " with ID " + std::to_string(id));
    }

    std::shared_ptr<Object> create_and_add_object(unsigned int id, const std::string &name)
    {
        std::shared_ptr<Object> new_obj = std::make_shared<Object>(id, name);
        add_object(id, new_obj); // Calls the thread-safe add_object
        return new_obj;
    }

    std::shared_ptr<Object> get_object(const std::string &name) const {
        // ParallelMap's get method returns std::optional
        std::optional<unsigned int> id_opt = name_to_id_map_.get(name);
        if (id_opt.has_value()) {
            return get_object(id_opt.value());
        }
        return nullptr;
    }
    std::shared_ptr<Object> get_object(unsigned int id) const
    {
        // ParallelMap's get method returns std::optional
        std::optional<std::shared_ptr<Object>> found_obj_opt = object_map_.get(id);
        if (found_obj_opt.has_value()) {
            return found_obj_opt.value();
        }
        return nullptr; // Or throw an exception, depending on your error handling
    }

    bool remove_object(unsigned int id)
    {
        // ParallelMap's remove method is thread-safe
        return object_map_.remove(id);
    }

    bool contains(unsigned int id) const
    {
        // ParallelMap's contains method is thread-safe
        return object_map_.contains(id);
    }

    size_t get_object_count() const
    {
        // ParallelMap's size method is thread-safe
        return object_map_.size();
    }

    // ... rest of your methods
    std::vector<std::shared_ptr<Object>> get_objects_snapshot() const
    {
        std::vector<std::shared_ptr<Object>> snapshot_vec;
        // Use ParallelMap's snapshot method to get a consistent view
        std::vector<ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Object>>::value_type> raw_snapshot = object_map_.snapshot();
        for (const auto& pair : raw_snapshot) {
            snapshot_vec.push_back(pair.second);
        }
        return snapshot_vec;
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
    ngin::debug::Logger* logger_;
    
    // Use ParallelMap directly
    ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Object>> object_map_;
    ngin::jobs::ParallelMap<std::string, unsigned int> name_to_id_map_;

    ModuleManager module_mgr_;

    ngin::jobs::ParallelQueue<ObjectEdit> edit_queue_;

    void execute_edit_(const ObjectEdit& edit) {
        auto obj = get_object(edit.object_id);
        if (!obj) {
            return;
        }
        switch (edit.kind) {
            case ObjectEditKind::EditParent: {
                obj->set_parent(edit.new_value);
                // readjust object hierarchy level too
                auto parent_obj = get_object(edit.new_value);
                if (parent_obj) {
                    obj->set_level(parent_obj->get_level() + 1);
                }
                break;
            }
            case ObjectEditKind::AddChild: {
                obj->add_child(edit.new_value);
                // inform child too
                auto child_obj = get_object(edit.new_value);
                if (child_obj) {
                    child_obj->set_parent(edit.object_id);
                    child_obj->set_level(obj->get_level() + 1);
                }
                break;
            }
            case ObjectEditKind::RemoveChild: {
                obj->remove_child(edit.new_value);
                // destroy child object
                remove_object(edit.new_value);
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