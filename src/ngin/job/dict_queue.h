#ifndef THREADABLE_DICT_QUEUE_H
#define THREADABLE_DICT_QUEUE_H

#include <vector>
#include <queue>
#include <map>
#include <string>
#include <mutex>
#include <utility> // For std::move

/**
 * @class ThreadableQueueDict
 * @brief A thread-safe dictionary where each key maps to a queue of values.
 * This implementation uses a single mutex to manage thread safety for the map and its queues.
 * @tparam K The type of the keys.
 * @tparam V The type of values in the queues.
 */
template<typename K, typename V>
class ThreadableQueueDict {
public:
    /**
     * @brief Pushes an item into the queue for the given key.
     * If the key does not exist, a new queue is created automatically.
     * @param key The key to associate the item with.
     * @param item The item to push into the queue.
     */
    void push(const K& key, V item) {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        m_map_[key].push(std::move(item));
    }

    /**
     * @brief Tries to pop a single item from the queue for a given key.
     * @param key The key of the queue to pop from.
     * @param item The variable to receive the popped item.
     * @return True if an item was popped, false if the queue does not exist or is empty.
     */
    bool try_pop(const K& key, V& item) {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        auto it = m_map_.find(key);
        if (it != m_map_.end() && !it->second.empty()) {
            item = std::move(it->second.front());
            it->second.pop();
            return true;
        }
        return false;
    }

    /**
     * @brief Pops all items from the queue for a given key and places them in the provided vector.
     * If the key does not exist, the vector remains unchanged.
     * @param key The key of the queue to pop items from.
     * @param items The vector to which the items will be added.
     */
    void pop_all(const K& key, std::vector<V>& items) {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        auto it = m_map_.find(key);
        if (it != m_map_.end()) {
            while (!it->second.empty()) {
                items.push_back(std::move(it->second.front()));
                it->second.pop();
            }
        }
    }

    /**
     * @brief Checks if the queue for a given key is empty.
     * @param key The key to check.
     * @return True if the queue is empty or if the key does not exist, false otherwise.
     */
    bool empty(const K& key) const {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        auto it = m_map_.find(key);
        if (it != m_map_.end()) {
            return it->second.empty();
        }
        return true;
    }

    /**
     * @brief Checks if all queues in the dictionary are empty.
     * @return True if all queues are empty or if the dictionary itself is empty.
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        for (const auto& pair : m_map_) {
            if (!pair.second.empty()) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Clears the queue for a specific key.
     * If the key does not exist, this function does nothing.
     * @param key The key of the queue to clear.
     */
    void clear(const K& key) {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        auto it = m_map_.find(key);
        if (it != m_map_.end()) {
            std::queue<V> empty_queue;
            it->second.swap(empty_queue);
        }
    }

    /**
     * @brief Clears all queues and removes all keys from the dictionary.
     */
    void clear_all() {
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        m_map_.clear();
    }

    /**
     * @brief Retrieves a list of all keys currently in the dictionary.
     * @return A std::vector containing all the keys.
     */
    std::vector<K> keys() const {
        std::vector<K> key_list;
        std::lock_guard<std::mutex> lock(m_map_mutex_);
        key_list.reserve(m_map_.size());
        for(const auto& pair : m_map_) {
            key_list.push_back(pair.first);
        }
        return key_list;
    }

private:
    std::map<K, std::queue<V>> m_map_;
    mutable std::mutex m_map_mutex_;
};

#endif // THREADABLE_DICT_QUEUE_H