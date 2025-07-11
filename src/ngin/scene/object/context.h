#ifndef OBJECT_CONTEXT_H
#define OBJECT_CONTEXT_H

#include <parallel_hashmap/phmap.h>

#include <mutex>        // For std::mutex
#include <shared_mutex> // For std::shared_mutex (often preferred for read-heavy)

#include <memory>   // For std::shared_ptr, std::unique_ptr
#include <string>   // For std::string
#include <iostream> // For print statements (example usage)
#include <optional> // For std::optional (safe return values)
#include <utility>  // For std::move, std::forward

#include <ngin/scene/object/object.h>
#include <ngin/job/collections/queue.h>

#include <ngin/debug/context.h>

#include <ngin/util/id.h>

namespace ngin
{
namespace scene
{

class ObjectContext
{
public:
    ObjectContext(ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Object>>& object_map, 
        ngin::jobs::ParallelMap<std::string, unsigned int>& name_to_id_map,
        ngin::jobs::ParallelQueue<ObjectEdit>& edit_queue, 
        ngin::debug::DebugContext& context) : object_map_(object_map),
                                                    name_to_id_map_(name_to_id_map), 
                                                    edit_queue_(edit_queue), logger_(context) {
    }
    ~ObjectContext() {
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

        logger_.info("Added object " + obj->get_name() + " with ID " + std::to_string(id));
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

    std::vector<std::shared_ptr<Object>> get_hierarchical_objects() const {
        std::vector<std::shared_ptr<Object>> all_objects = get_objects_snapshot();

        // Map to quickly find objects by ID
        phmap::flat_hash_map<unsigned int, std::shared_ptr<Object>> id_to_object_map;
        for (const auto& obj : all_objects) {
            id_to_object_map[obj->get_id()] = obj;
        }

        // Map to store children for each parent
        phmap::flat_hash_map<unsigned int, std::vector<std::shared_ptr<Object>>> children_map;
        std::vector<std::shared_ptr<Object>> root_objects;

        for (const auto& obj : all_objects) {
            if (obj->get_parent() == 0) { // Assuming 0 is the root parent ID
                root_objects.push_back(obj);
            } else {
                children_map[obj->get_parent()].push_back(obj);
            }
        }

        // Sort children by level for consistent traversal (optional, but good for display)
        for (auto& pair : children_map) {
            std::sort(pair.second.begin(), pair.second.end(), [](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) {
                return a->get_level() < b->get_level();
            });
        }

        std::vector<std::shared_ptr<Object>> ordered_objects;
        std::function<void(std::shared_ptr<Object>)> traverse =
            [&](std::shared_ptr<Object> current_obj) {
            ordered_objects.push_back(current_obj);
            if (children_map.count(current_obj->get_id())) {
                for (const auto& child : children_map[current_obj->get_id()]) {
                    traverse(child);
                }
            }
        };

        // Start traversal from root objects
        std::sort(root_objects.begin(), root_objects.end(), [](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) {
            return a->get_level() < b->get_level();
        });

        for (const auto& root : root_objects) {
            traverse(root);
        }

        return ordered_objects;
    }

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

private:
    ngin::jobs::ParallelMap<unsigned int, std::shared_ptr<Object>>& object_map_;
    ngin::jobs::ParallelMap<std::string, unsigned int>& name_to_id_map_;    
    ngin::jobs::ParallelQueue<ObjectEdit>& edit_queue_;
    ngin::debug::DebugContext& logger_;
};

}
}

#endif // OBJECT_CONTEXT_H
