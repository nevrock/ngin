#ifndef THREADABLE_VECTOR_H
#define THREADABLE_VECTOR_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm> // For std::remove_if, std::find, etc.

namespace ngin {
namespace jobs {

template<typename T>
class ParallelVector {
public:
    // Adds an item to the end of the vector.
    void push_back(T item) {
        {
            std::lock_guard<std::mutex> lock(m_mutex_);
            m_vector_.push_back(std::move(item));
        }
        m_cv_.notify_one(); // Notify waiting threads that an item has been added
    }

    // Tries to pop the last item from the vector.
    // Returns true if an item was successfully popped, false if the vector was empty.
    bool try_pop_back(T& item) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        if (m_vector_.empty()) {
            return false;
        }
        item = std::move(m_vector_.back());
        m_vector_.pop_back();
        return true;
    }

    // Tries to pop the first item from the vector.
    // Returns true if an item was successfully popped, false if the vector was empty.
    bool try_pop_front(T& item) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        if (m_vector_.empty()) {
            return false;
        }
        item = std::move(m_vector_.front());
        m_vector_.erase(m_vector_.begin()); // Erase the first element
        return true;
    }

    // Atomically appends all items from a given vector to the end of this vector.
    void push_back_all(std::vector<T>& items) {
        {
            std::lock_guard<std::mutex> lock(m_mutex_);
            m_vector_.insert(m_vector_.end(), std::make_move_iterator(items.begin()), std::make_move_iterator(items.end()));
            items.clear(); // Clear the source vector after moving elements
        }
        m_cv_.notify_all(); // Notify all waiting threads as multiple items might have been added
    }

    // Removes all items from the vector and moves them into the provided vector.
    void pop_all(std::vector<T>& items) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        items.reserve(items.size() + m_vector_.size()); // Pre-allocate memory
        std::move(m_vector_.begin(), m_vector_.end(), std::back_inserter(items));
        m_vector_.clear();
    }

    // Checks if the vector is empty.
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        return m_vector_.empty();
    }

    // Returns the current size of the vector.
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        return m_vector_.size();
    }

    // Clears all elements from the vector.
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_vector_.clear();
    }

    // Accesses an element at a specific index.
    // This operation should be used with caution in a multithreaded context,
    // as the index might become invalid or the element might be removed by another thread.
    // It's generally safer to pop elements or iterate over a copied snapshot.
    T at(size_t index) const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        if (index >= m_vector_.size()) {
            throw std::out_of_range("ParallelVector::at index out of range");
        }
        return m_vector_.at(index);
    }

    // Removes the first occurrence of a specific item from the vector.
    // Returns true if the item was found and removed, false otherwise.
    bool remove_one(const T& item) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        auto it = std::find(m_vector_.begin(), m_vector_.end(), item);
        if (it != m_vector_.end()) {
            m_vector_.erase(it);
            return true;
        }
        return false;
    }

    // Removes all occurrences of a specific item from the vector.
    // Returns the number of items removed.
    size_t remove_all(const T& item) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        size_t initial_size = m_vector_.size();
        m_vector_.erase(std::remove(m_vector_.begin(), m_vector_.end(), item), m_vector_.end());
        return initial_size - m_vector_.size();
    }

    // Allows for iteration over a *copy* of the vector. This is important
    // for thread safety when iterating, as the underlying vector might change.
    std::vector<T> get_copy() const {
        std::lock_guard<std::mutex> lock(m_mutex_);
        return m_vector_;
    }

private:
    std::vector<T> m_vector_;
    mutable std::mutex m_mutex_;
    std::condition_variable m_cv_; // Used for signaling, similar to the queue
};

}
}

#endif