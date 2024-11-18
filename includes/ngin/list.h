#ifndef LIST_H
#define LIST_H

#include <ngin/collection.h>
#include <vector>
#include <any>
#include <algorithm>
#include <stdexcept>

class List : public Collection {
public:
    // Default constructor
    List() = default;

    // Destructor
    virtual ~List() = default;

    // Copy constructor
    List(const List& other) : data(other.data) {}

    // Move constructor
    List(List&& other) noexcept : data(std::move(other.data)) {}

    // Copy assignment operator
    List& operator=(const List& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }

    // Move assignment operator
    List& operator=(List&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }

    // Add an element
    void add(const std::any& element) {
        data.push_back(element);
    }

    // Clear all elements
    void clear() {
        data.clear();
    }

    // Get an element by index
    template <typename T>
    T get(size_t index) const {
        if (index < data.size()) {
            return std::any_cast<T>(data[index]);
        }
        throw std::out_of_range("Index out of range");
    }

    // Get the length of the list
    size_t getLength() const {
        return data.size();
    }

    // Remove an element (removes the first matching element)
    bool remove(const std::any& element) {
        auto it = std::find_if(data.begin(), data.end(), [&](const std::any& el) {
            return compareAny(el, element);
        });
        if (it != data.end()) {
            data.erase(it);
            return true;
        }
        return false;
    }

    // Check if an element exists
    bool contains(const std::any& element) const {
        return std::any_of(data.begin(), data.end(), [&](const std::any& el) {
            return compareAny(el, element);
        });
    }

    // Compare two std::any elements
    static bool compareAny(const std::any& a, const std::any& b) {
        if (a.type() != b.type()) return false;

        if (a.type() == typeid(int)) {
            return std::any_cast<int>(a) == std::any_cast<int>(b);
        } else if (a.type() == typeid(float)) {
            return std::any_cast<float>(a) == std::any_cast<float>(b);
        } else if (a.type() == typeid(std::string)) {
            return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
        } else if (a.type() == typeid(std::vector<std::any>)) {
            const auto& vecA = std::any_cast<const std::vector<std::any>&>(a);
            const auto& vecB = std::any_cast<const std::vector<std::any>&>(b);
            if (vecA.size() != vecB.size()) return false;
            for (size_t i = 0; i < vecA.size(); ++i) {
                if (!compareAny(vecA[i], vecB[i])) return false;
            }
            return true;
        }
        return false;
    }

    // Get the data vector
    const std::vector<std::any>& getData() const {
        return data;
    }

private:
    std::vector<std::any> data;
};

#endif // LIST_H
