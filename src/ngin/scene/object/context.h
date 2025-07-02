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

#include <ngin/util/id.h>

namespace ngin
{
namespace scene
{
using ObjectAtlas = phmap::parallel_flat_hash_map<
    unsigned int,
    std::shared_ptr<Object>,
    std::hash<unsigned int>,
    std::equal_to<unsigned int>,
    std::allocator<std::pair<const unsigned int, std::shared_ptr<Object>>>,
    8,
    std::shared_mutex // Use std::shared_mutex for internal shard locking
    >;
class ObjectContext
{
public:
    ObjectContext(ObjectAtlas& object_map, ngin::jobs::ParallelQueue<ObjectEdit>& edit_queue) : object_map_(object_map), edit_queue_(edit_queue) {

    }

    void add_object(unsigned int id, std::shared_ptr<Object> obj)
    {
        // phmap::parallel_flat_hash_map is thread-safe for these operations.
        // It uses its internal shard locks.
        object_map_.lazy_emplace_l(id,
                                   [&](ObjectAtlas::value_type &val) { val.second = obj; },
                                   [&](const ObjectAtlas::constructor &ctor) { ctor(id, obj); });
    }

    std::shared_ptr<Object> create_and_add_object(unsigned int id, const std::string &name)
    {
        std::shared_ptr<Object> new_obj = std::make_shared<Object>(id, name);
        add_object(id, new_obj); // Calls the thread-safe add_object
        return new_obj;
    }

    std::shared_ptr<Object> get_object(unsigned int id) const
    {
        std::shared_ptr<Object> found_obj;
        // if_contains is thread-safe
        object_map_.if_contains(id, [&](const ObjectAtlas::value_type &pair)
                               { found_obj = pair.second; });
        return found_obj;
    }

    bool remove_object(unsigned int id)
    {
        // erase is thread-safe
        return object_map_.erase(id);
    }

    bool contains(unsigned int id) const
    {
        // count is thread-safe
        return object_map_.count(id) > 0;
    }

    size_t get_object_count() const
    {
        // size() is thread-safe for parallel_flat_hash_map
        return object_map_.size();
    }

private:
    ObjectAtlas& object_map_;
    ngin::jobs::ParallelQueue<ObjectEdit>& edit_queue_;
};

}
}

#endif // OBJECT_CONTEXT_H
