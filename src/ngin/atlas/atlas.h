#ifndef ATLAS_H
#define ATLAS_H

#include <fstream>
#include <map>
#include <any>
#include <string>
#include <sstream>
#include <stack>
#include <algorithm>
#include <optional>
#include <iostream>
#include <vector>
#include <tuple> // Required for std::tuple

#include <glm/glm.hpp>
#include <unordered_map>

#include <ngin/assets/asset.h>
#include <ngin/debug/logger.h>

class Atlas : public Asset {
public:
    // CONSTRUCTORS
    Atlas(std::string name) : Asset(name), data_(new std::unordered_map<std::string, std::any>), keyOrder_(new std::vector<std::string>) { 
    }
    Atlas() : Asset(""), data_(new std::unordered_map<std::string, std::any>), keyOrder_(new std::vector<std::string>) {}
    Atlas(const Atlas& other) : Asset(""), data_(new std::unordered_map<std::string, std::any>(*other.data_)), keyOrder_(new std::vector<std::string>(*other.keyOrder_)) {}
    Atlas& operator=(const Atlas& other) {
        if (this != &other) {
            *data_ = *other.data_;
            *keyOrder_ = *other.keyOrder_;
        }
        return *this;
    }
    Atlas(Atlas&& other) noexcept : Asset(""), data_(other.data_), keyOrder_(other.keyOrder_) {
        other.data_ = nullptr;
        other.keyOrder_ = nullptr;
    }
    Atlas& operator=(Atlas&& other) noexcept {
        if (this != &other) {
            delete data_;
            delete keyOrder_;
            data_ = other.data_;
            keyOrder_ = other.keyOrder_;
            other.data_ = nullptr;
            other.keyOrder_ = nullptr;
        }
        return *this;
    }
    ~Atlas() {
        delete data_;
        delete keyOrder_;
    }

    template<typename T>
    void set(const std::string& key, T&& value) { // Using forwarding reference
        if (data_->find(key) == data_->end()) {
            keyOrder_->push_back(key); // Track insertion order
        }
        (*data_)[key] = std::forward<T>(value);
    }
    template<typename T>
    T* get(const std::string& key, T* defaultValue = nullptr) {
        auto it = data_->find(key);
        if (it != data_->end() && it->second.type() == typeid(T)) {
            return std::any_cast<T>(&it->second);
        }
        return defaultValue; // Return the default value if the key is not found or type mismatch
    }
    template<typename T>
    const T* get(const std::string& key, const T* defaultValue = nullptr) const {
        auto it = data_->find(key);
        if (it != data_->end() && it->second.type() == typeid(T)) {
            return std::any_cast<const T>(&it->second);
        }
        return defaultValue; // Return the default value if the key is not found or type mismatch
    }

    const std::any& get(const std::string& key) const {
        auto it = data_->find(key);
        if (it != data_->end()) {
            return it->second;
        }
        throw std::out_of_range("Key not found: " + key);
    }
    
    bool istype(const std::string& key, const std::type_info& type) const {
        auto it = data_->find(key);
        if (it == data_->end()) {
            return false;
        }
        return it->second.type() == type;
    }
    std::string gettype(const std::string& key) const {
        auto it = data_->find(key);
        if (it == data_->end()) {
            return "none"; // Return "none" if the key is not found
        }
        const std::type_info& type = it->second.type();
        if (type == typeid(int)) {
            return "int";
        } else if (type == typeid(float)) {
            return "float";
        } else if (type == typeid(bool)) {
            return "bool";
        } else if (type == typeid(std::string)) {
            return "string";
        } else if (type == typeid(std::vector<int>)) {
            return "vector_int";
        } else if (type == typeid(std::vector<float>)) {
            return "vector_float";
        } else if (type == typeid(std::vector<bool>)) {
            return "vector_bool";
        } else if (type == typeid(std::vector<std::string>)) {
            return "vector_string";
        } else if (type == typeid(Atlas)) {
            return "atlas";
        } else {
            return "unknown"; // Return "unknown" for any other type
        }
    }
    bool contains(const std::string& key) const {
        return data_->find(key) != data_->end();
    }
    bool has(const std::string& key) const {
        return contains(key);
    }
    void sync(const Atlas* other, bool overwrite = false) {
        if (other == nullptr) {
            std::cerr << "provided dictionary pointer is null" << std::endl;
            return;
        }
        for (const auto& pair : *other->data_) {
            auto it = data_->find(pair.first);
            if (it != data_->end()) {
                if (overwrite) {
                    it->second = pair.second;  // Overwrite the existing entry with the new value
                }
            } else {
                (*data_)[pair.first] = pair.second;  // Add new entry if it does not exist
                keyOrder_->push_back(pair.first);
            }
        }
    }
    void removeat(const std::string& key) {
        auto it = data_->find(key);
        if (it != data_->end()) {
            data_->erase(it);
            keyOrder_->erase(std::remove(keyOrder_->begin(), keyOrder_->end(), key), keyOrder_->end());
        }
    }
    void read(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        std::unordered_map<int, Atlas*> dictStack;
        dictStack[-1] = this;

        std::string collectionName = std::string("");

        for (std::string line; getline(file, line); ) {
            if (line.empty() || (line[0] == '/' && line[1] == '/')) continue;

            int indent = line.find_first_not_of(' ');
            indent = indent / 4;

            line = trim_(line);
            int delimiterPos = line.find(':');
            if (delimiterPos == (int)std::string::npos) {
                continue;
            } else {
                std::string key = line.substr(0, delimiterPos);
                std::string value = delimiterPos < (int)line.size() - 1 ? trim_(line.substr(delimiterPos + 1)) : "";
                if (value == "") {
                    // could be a list of a dict
                    dictStack[indent - 1]->set(key, *(new Atlas()));
                    dictStack[indent] = dictStack[indent - 1]->get<Atlas>(key);

                } else {
                    parse_atlas_element_(dictStack[indent - 1], key, value);
                }
            }

            // currentIndent = indent;
        }
    }
    void write(const std::string& filepath) const {
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "failed to open file for writing: " << filepath << std::endl;
            return;
        }

        file << get_string_();

        file.close();
    }
    void clear() {
        data_->clear();
        keyOrder_->clear();
    }
    size_t length() const {
        return data_->size();
    }

    void log_keys(Logger& logger, std::string name = "") const {
        std::string logm = "Keys: [ ";
        if (name != "") {
            logm = name + " - " + logm;
        }
        for (const auto& key : *keyOrder_) {
            logm += key + ", ";
        }
        logm += "]";
        logger.info(logm);
    }

    // ITERATORS
    std::unordered_map<std::string, std::any>::iterator raw_begin() {
        return data_->begin();
    }
    std::unordered_map<std::string, std::any>::iterator raw_end() {
        return data_->end();
    }
    std::unordered_map<std::string, std::any>::const_iterator raw_begin() const{
        return data_->cbegin();
    }
    std::unordered_map<std::string, std::any>::const_iterator raw_end() const {
        return data_->cend();
    }
    std::vector<std::string> keys() const {
        return *keyOrder_; // Return keys in insertion order
    }
    
    class Iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::pair<const std::string, const std::any&>;
            using pointer = const value_type*;
            using reference = const value_type&;

            Iterator(const std::unordered_map<std::string, std::any>& data, const std::vector<std::string>& keyOrder, size_t index)
                : data_(data), keyOrder_(keyOrder), index_(index) {}

            reference operator*() const {
                const std::string& key = keyOrder_[index_];
                auto it = data_.find(key);
                return *it;
            }

            pointer operator->() const { return &(this->operator*()); }

            Iterator& operator++() {
                ++index_;
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const {
                return index_ == other.index_;
            }

            bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

        private:
            const std::unordered_map<std::string, std::any>& data_;
            const std::vector<std::string>& keyOrder_;
            size_t index_;
    };
    
    Iterator begin() const {
        return Iterator(*data_, *keyOrder_, 0);
    }
    Iterator end() const {
        return Iterator(*data_, *keyOrder_, keyOrder_->size());
    }
    std::any& operator[](const std::string& key) {
        return (*data_)[key];
    }
    const std::any& operator[](const std::string& key) const {
        auto it = data_->find(key);
        if (it != data_->end()) {
            return it->second;
        }
        throw std::out_of_range("Key not found: " + key);
    }

    const std::unordered_map<std::string, std::any>& data() const { return *data_; }
    const std::unordered_map<std::string, std::any>& getmap() const {
        return *data_;
    }

private:
    std::unordered_map<std::string, std::any>* data_;
    std::vector<std::string>* keyOrder_; // Stores keys in insertion order

    std::string trim_(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    bool starts_with_(const std::string& str, const std::string& prefix) {
        if (str.size() < prefix.size()) {
            return false;
        }
        return str.compare(0, prefix.size(), prefix) == 0;
    }
    std::string get_string_(int indent = 0) const {
        std::string result;
        for (const auto& key : *keyOrder_) {
            auto it = data_->find(key);
            if (it != data_->end()) {
                result += std::string(indent, ' ') + it->first + ": ";
                if (it->second.type() == typeid(Atlas)) {
                    result += "\n";
                    result += std::any_cast<const Atlas&>(it->second).get_string_(indent + 4);
                } else if (it->second.type() == typeid(std::vector<float>) ||
                           it->second.type() == typeid(std::vector<int>) ||
                           it->second.type() == typeid(std::vector<std::string>) ||
                           it->second.type() == typeid(std::vector<bool>)) {
                    result += "[";
                    bool first = true;
                    if (auto v = std::any_cast<std::vector<int>>(&it->second)) {
                        for (int e : *v) {
                            if (!first) result += ", ";
                            result += std::to_string(e);
                            first = false;
                        }
                    } else if (auto v = std::any_cast<std::vector<float>>(&it->second)) {
                        for (float e : *v) {
                            if (!first) result += ", ";
                            result += std::to_string(e);
                            first = false;
                        }
                    } else if (auto v = std::any_cast<std::vector<std::string>>(&it->second)) {
                        for (const std::string& e : *v) {
                            if (!first) result += ", ";
                            result += '"' + e + '"';
                            first = false;
                        }
                    } else if (auto v = std::any_cast<std::vector<bool>>(&it->second)) {
                        for (bool e : *v) {
                            if (!first) result += ", ";
                            result += e ? "true" : "false";
                            first = false;
                        }
                    }
                    result += "]\n";
                } else if (it->second.type() == typeid(int)) {
                    result += std::to_string(std::any_cast<int>(it->second)) + "\n";
                } else if (it->second.type() == typeid(float)) {
                    result += std::to_string(std::any_cast<float>(it->second)) + "\n";
                } else if (it->second.type() == typeid(std::string)) {
                    result += '"' + std::any_cast<std::string>(it->second) + "\"\n";
                } else if (it->second.type() == typeid(bool)) {
                    result += std::any_cast<bool>(it->second) ? "true\n" : "false\n";
                }
            }
        }
        return result;
    }
    std::any parse_value_(const std::string& value) {
        std::string type = Atlas::get_type_(value);
        if (value.empty()) {
            return {};
        } else if (type == "color") { // Parse #RRGGBB as vector_float
            std::vector<float> color(4, 1.0f); // Default alpha to 1.0
            try {
                color[0] = std::stoi(value.substr(1, 2), nullptr, 16) / 255.0f; // Red
                color[1] = std::stoi(value.substr(3, 2), nullptr, 16) / 255.0f; // Green
                color[2] = std::stoi(value.substr(5, 2), nullptr, 16) / 255.0f; // Blue
            } catch (...) {
                return {}; // Return empty on error
            }
            return color;
        } else if (starts_with_(type, "vector_")) {
            if (type == "vector_float") {
                std::vector<float> vecFloat;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    vecFloat.push_back(std::stof(trim_(item)));
                }
                return vecFloat;
            } else if (type == "vector_int") {
                std::vector<int> vecInt;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    vecInt.push_back(std::stoi(trim_(item)));
                }
                return vecInt;
            } else if (type == "vector_bool") {
                std::vector<bool> vecBool;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    if (trim_(item) == "true") {
                        vecBool.push_back(true);
                    } else {
                        vecBool.push_back(false);
                    }
                }
                return vecBool;
            } else if (type == "vector_string") {
                std::vector<std::string> vecString;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    std::string itemS = trim_(item);
                    itemS.erase(std::remove(itemS.begin(), itemS.end(), '"'), itemS.end());
                    itemS.erase(std::remove(itemS.begin(), itemS.end(), '\''), itemS.end());
                    vecString.push_back(itemS);
                }
                return vecString;
            }
        } else if (type == "int") {
            return std::stoi(value);
        } else if (type == "float") {
            return std::stof(value);
        } else if (type == "bool") {
            if (value == "true") { return true; }
            else { return false; }
        } else {
            if (value.front() == '"' && value.back() == '"') {
                return value.substr(1, value.length() - 2);
            } else if (value.front() == '\'' && value.back() == '\'') {
                return value.substr(1, value.length() - 2);
            } else {
                return value;
            }
        }
        return {}; // Add a default return value to avoid warnings
    }
    std::string get_type_(const std::string& value) {
        if (value.empty()) {
            return "";
        } else if (value.front() == '#' && value.size() == static_cast<std::string::size_type>(7)) {
            return "color";
        } else if (value.front() == '[' && value.back() == ']') {
            if (value.size() >= 2) {
                std::stringstream ss(value.substr(1, value.size() - 2));
                std::string item;
                std::string vectorType = "";
                while (getline(ss, item, ',')) {
                    vectorType = Atlas::get_type_(trim_(item));
                    break;
                }
                return "vector_"+vectorType;
            } else {
                return "string"; // Or handle this case as an error if appropriate
            }
        } else if (value.front() == '"' && value.back() == '"') {
            return "string";
        } else if (value.front() == '\'' && value.back() == '\'') {
            return "string";
        } else if (std::all_of(value.begin(), value.end(), [](char c) { return ::isdigit(c) || c == '-'; }) &&
                (value.front() == '-' ? value.size() > 1 : true)) {
            return "int";
        } else if ((value.front() == '-' && std::count_if(value.begin() + 1, value.end(), [](char c) { return ::isdigit(c) || c == '.'; }) == (int)value.size() - 1 && std::count(value.begin() + 1, value.end(), '.') <= 1) ||
                (std::count_if(value.begin(), value.end(), [](char c) { return ::isdigit(c) || c == '.'; }) == (int)value.size() && std::count(value.begin(), value.end(), '.') <= 1)) {
            return "float";
        } else if (value == "true" || value == "false") {
            return "bool";
        } else {
            return "string";
        }
    }
    void parse_atlas_element_(Atlas* dict, std::string& key, std::string& value) {
        dict->set(key, parse_value_(value));
    }
    Atlas* parse_atlas_(Atlas* dict, std::string& collectionName) {
        Atlas* newAtlas = new Atlas();
        dict->set(collectionName, *newAtlas);
        return newAtlas;
    }
};

#endif // ATLAS_H
