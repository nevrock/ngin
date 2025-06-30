#ifndef THREADABLE_DICT_H
#define THREADABLE_DICT_H

#include <map>
#include <mutex>
#include <shared_mutex> // For std::shared_mutex (C++17 for read-write locks)
#include <utility>      // For std::forward

/**
 * @class ThreadableDict
 * @brief A thread-safe dictionary (map) implementation.
 *
 * This class provides a basic thread-safe wrapper around std::map,
 * protecting access to its underlying data using a std::shared_mutex.
 * This allows for multiple concurrent readers but exclusive access for writers.
 *
 * @tparam Key The type of the keys in the dictionary.
 * @tparam Value The type of the values in the dictionary.
 */
template<typename Key, typename Value>
class ThreadableDict {
public:
    /**
     * @brief Default constructor.
     */
    ThreadableDict() = default;

    /**
     * @brief Inserts or updates an element in the dictionary.
     *
     * This operation is thread-safe. If the key already exists, its value is updated.
     *
     * @param key The key to insert or update.
     * @param value The value to associate with the key.
     */
    void set(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_); // Acquire unique lock for write
        data_[key] = value;
    }

    /**
     * @brief Inserts or updates an element using rvalue references.
     *
     * This operation is thread-safe and allows for efficient moving of key/value pairs.
     *
     * @param key An rvalue reference to the key.
     * @param value An rvalue reference to the value.
     */
    void set(Key&& key, Value&& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_); // Acquire unique lock for write
        data_[std::forward<Key>(key)] = std::forward<Value>(value);
    }

    /**
     * @brief Retrieves a copy of the value associated with a given key.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @param key The key of the element to retrieve.
     * @param out_value A reference to where the retrieved value will be stored.
     * @return True if the key was found and the value was retrieved, false otherwise.
     */
    bool get(const Key& key, Value& out_value) const {
        std::shared_lock<std::shared_mutex> lock(mutex_); // Acquire shared lock for read
        auto it = data_.find(key);
        if (it != data_.end()) {
            out_value = it->second;
            return true;
        }
        return false;
    }

    /**
     * @brief Checks if the dictionary contains a specific key.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @param key The key to check for.
     * @return True if the key exists in the dictionary, false otherwise.
     */
    bool contains(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_); // Acquire shared lock for read
        return data_.count(key) > 0;
    }

    /**
     * @brief Removes an element from the dictionary.
     *
     * This operation is thread-safe.
     *
     * @param key The key of the element to remove.
     * @return True if the element was successfully removed, false if the key was not found.
     */
    bool remove(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_); // Acquire unique lock for write
        return data_.erase(key) > 0;
    }

    /**
     * @brief Returns the number of elements in the dictionary.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @return The number of elements.
     */
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_); // Acquire shared lock for read
        return data_.size();
    }

    /**
     * @brief Checks if the dictionary is empty.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @return True if the dictionary is empty, false otherwise.
     */
    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex_); // Acquire shared lock for read
        return data_.empty();
    }

    /**
     * @brief Clears all elements from the dictionary.
     *
     * This operation is thread-safe.
     */
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_); // Acquire unique lock for write
        data_.clear();
    }

    /**
     * @brief Retrieves a vector of all keys currently in the dictionary.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @return A std::vector containing all keys.
     */
    std::vector<Key> keys() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<Key> all_keys;
        for (const auto& pair : data_) {
            all_keys.push_back(pair.first);
        }
        return all_keys;
    }

    /**
     * @brief Retrieves a vector of all values currently in the dictionary.
     *
     * This operation is thread-safe and allows multiple concurrent readers.
     *
     * @return A std::vector containing all values.
     */
    std::vector<Value> values() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<Value> all_values;
        for (const auto& pair : data_) {
            all_values.push_back(pair.second);
        }
        return all_values;
    }

private:
    std::map<Key, Value> data_;
    mutable std::shared_mutex mutex_; // Use shared_mutex for read-write locking
};

#endif // THREADABLE_DICT_H

