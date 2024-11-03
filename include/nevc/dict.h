#ifndef DICT_H
#define DICT_H

#include <fstream>
#include <map>
#include <any>
#include <string>
#include <sstream>
#include <stack>
#include <algorithm>
#include <optional>

#include <nevc/collection.h>
#include <nevc/list.h>

namespace nevc {

class Dict : public nevc::Collection {
public:
    Dict() = default;
    ~Dict() = default;
    Dict(const Dict& other) {
        data_ = other.data_; // Use the map's copy assignment
    }
    Dict& operator=(const Dict& other) {
        if (this != &other) {
            data_ = other.data_; // Use the map's copy assignment
        }
        return *this;
    }
    Dict(Dict&& other) noexcept {
        data_ = std::move(other.data_);
    }
    Dict& operator=(Dict&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }

    void set(const std::string& key, std::any value) {
        data_[key] = value;
    }
    template<typename T>
    T* get(const std::string& key, T* defaultValue = nullptr) {
        // std::cout << "dict search - start " << key << std::endl;
        auto it = data_.find(key);
        if (it != data_.end() && it->second.type() == typeid(T)) {
            // std::cout << "dict search - found with key " << key << std::endl;
            return std::any_cast<T>(&it->second);
        }
        // std::cout << "dict search - failed with key " << key << std::endl;
        return defaultValue; // Return the default value if the key is not found or type mismatch
    }
    template<typename T>
    const T* get(const std::string& key, const T* defaultValue = nullptr) const {
        // std::cout << "dict search - start " << key << std::endl;
        auto it = data_.find(key);
        if (it != data_.end() && it->second.type() == typeid(T)) {
            // std::cout << "dict search - found with key " << key << std::endl;
            return std::any_cast<const T>(&it->second);
        }
        // std::cout << "dict search - failed with key " << key << std::endl;
        return defaultValue; // Return the default value if the key is not found or type mismatch
    }
    template<typename T>
    T getC(const std::string& key, const T& defaultValue) const {
        auto it = data_.find(key);
        if (it != data_.end() && it->second.type() == typeid(T)) {
            return std::any_cast<T>(it->second);
        }
        return defaultValue;
    }
    glm::vec3 getVec(const std::string& key, const glm::vec3& defaultValue) const {
        std::vector<float> l = getC<std::vector<float>>(key, std::vector<float>{0.0,0.0,0.0});
        if (l.size() != 3) {
            std::cout << "dict failed to convert to vec - list length mismatch" << std::endl;
            return defaultValue; // Return default if the length is not exactly 3
        }
        glm::vec3 vec(0.0f);
        try {
            for (int i = 0; i < 3; ++i) {
                vec[i] = l[i];
            }
        } catch (const std::bad_any_cast& e) {
            try {
                std::vector<int> li = getC<std::vector<int>>(key, std::vector<int>{0,0,0});
                for (int i = 0; i < 3; ++i) {
                    vec[i] = li[i];
                }
            } catch (const std::bad_any_cast& e2) {
                std::cout << "dict failed to convert to vec - bad any cast" << std::endl;
                return defaultValue; // Return default if the index is out of range
            } catch (const std::out_of_range& e2) {
                std::cout << "dict failed to convert to vec - out of range" << std::endl;
                return defaultValue; // Return default if the index is out of range
            }
            return vec; // Return default if there's a type mismatch
        } catch (const std::out_of_range& e) {
            std::cout << "dict failed to convert to vec - out of range" << std::endl;
            return defaultValue; // Return default if the index is out of range
        }
        return vec;
    }
    glm::vec2 getVec2(const std::string& key, const glm::vec2& defaultValue) const {
        std::vector<float> l = getC<std::vector<float>>(key, std::vector<float>{0.0,0.0});
        if (l.size() != 2) {
            std::cout << "dict failed to convert to vec - list length mismatch" << std::endl;
            return defaultValue; // Return default if the length is not exactly 3
        }
        glm::vec2 vec(0.0f);
        try {
            for (int i = 0; i < 2; ++i) {
                vec[i] = l[i];
            }
        } catch (const std::bad_any_cast& e) {
            try {
                std::vector<int> li = getC<std::vector<int>>(key, std::vector<int>{0,0});
                for (int i = 0; i < 2; ++i) {
                    vec[i] = li[i];
                }
            } catch (const std::bad_any_cast& e2) {
                std::cout << "dict failed to convert to vec - bad any cast" << std::endl;
                return defaultValue; // Return default if the index is out of range
            } catch (const std::out_of_range& e2) {
                std::cout << "dict failed to convert to vec - out of range" << std::endl;
                return defaultValue; // Return default if the index is out of range
            }
            return vec; // Return default if there's a type mismatch
        } catch (const std::out_of_range& e) {
            std::cout << "dict failed to convert to vec - out of range" << std::endl;
            return defaultValue; // Return default if the index is out of range
        }
        return vec;
    }
    std::string getString(int indent = 0) const {
        std::string result;
        for (const auto& pair : data_) {
            result += std::string(indent, ' ') + pair.first + ": ";
            if (pair.second.type() == typeid(Dict)) {
                result += "\n";
                result += std::any_cast<const Dict&>(pair.second).getString(indent + 4);
            } else if (pair.second.type() == typeid(std::vector<float>) ||
                    pair.second.type() == typeid(std::vector<int>) ||
                    pair.second.type() == typeid(std::vector<std::string>) ||
                    pair.second.type() == typeid(std::vector<bool>)) {
                result += "[";
                bool first = true;
                if (auto v = std::any_cast<std::vector<int>>(&pair.second)) {
                    for (int e : *v) {
                        if (!first) result += ", ";
                        result += std::to_string(e);
                        first = false;
                    }
                } else if (auto v = std::any_cast<std::vector<float>>(&pair.second)) {
                    for (float e : *v) {
                        if (!first) result += ", ";
                        result += std::to_string(e);
                        first = false;
                    }
                } else if (auto v = std::any_cast<std::vector<std::string>>(&pair.second)) {
                    for (const std::string& e : *v) {
                        if (!first) result += ", ";
                        result += '"' + e + '"';
                        first = false;
                    }
                } else if (auto v = std::any_cast<std::vector<bool>>(&pair.second)) {
                    for (bool e : *v) {
                        if (!first) result += ", ";
                        result += e ? "true" : "false";
                        first = false;
                    }
                }
                result += "]\n";
            } else if (pair.second.type() == typeid(int)) {
                result += std::to_string(std::any_cast<int>(pair.second)) + "\n";
            } else if (pair.second.type() == typeid(float)) {
                result += std::to_string(std::any_cast<float>(pair.second)) + "\n";
            } else if (pair.second.type() == typeid(std::string)) {
                result += '"' + std::any_cast<std::string>(pair.second) + "\"\n";
            } else if (pair.second.type() == typeid(bool)) {
                result += std::any_cast<bool>(pair.second) ? "true\n" : "false\n";
            }
        }
        return result;
    }
    bool isType(const std::string& key, const std::type_info& type) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return false;
        }
        return it->second.type() == type;
    }
    bool contains(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    const std::map<std::string, std::any>& data() const { return data_; }
    void sync(const Dict* other, bool overwrite = false) {
        if (other == nullptr) {
            std::cerr << "Provided dictionary pointer is null" << std::endl;
            return;
        }
        for (const auto& pair : other->data()) {
            auto it = data_.find(pair.first);
            if (it != data_.end()) {
                if (overwrite) {
                    it->second = pair.second;  // Overwrite the existing entry with the new value
                }
            } else {
                data_[pair.first] = pair.second;  // Add new entry if it does not exist
            }
        }
    }
    void removeAt(const std::string& key) {
        auto it = data_.find(key);
        if (it != data_.end()) {
            data_.erase(it);
        }
    }
    void print(int indent = 0) {
        std::cout << "## dict print! ##" << std::endl;
        std::cout << getString(indent) << std::endl;
    }
    void read(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        std::map<int, Dict*> dictStack;
        dictStack[-1] = this;

        //std::stack<Dict*> dictStack;
        //dictStack.push(this);

        //int currentIndent = -1;

        if (isLog_) std::cout << "### dict parsing ### - " << filename << std::endl;

        std::string collectionName = std::string("");

        for (std::string line; getline(file, line); ) {
            if (line.empty() || (line[0] == '/' && line[1] == '/')) continue;

            if (isLog_) std::cout << line << std::endl;
            
            int indent = line.find_first_not_of(' ');
            indent = indent / 4;
            /*
            if (indent < currentIndent) {
                int levelsUp = (currentIndent - indent) / 4;
                for (int i = 0; i < levelsUp; ++i) {
                    if (!dictStack.empty()) {
                        dictStack.pop(); // Move up in the dictionary stack
                        if (isLog_) std::cout << "# moving up from nested dict" << std::endl;
                    }
                }
            }
            */

            if (isLog_) std::cout << "# indent - " << indent << std::endl;

            line = trim(line);
            int delimiterPos = line.find(':');
            if (delimiterPos == std::string::npos) {
                continue;
            } else {                
                std::string key = line.substr(0, delimiterPos);
                std::string value = delimiterPos < line.size() - 1 ? trim(line.substr(delimiterPos + 1)) : "";
                if (value == "") {
                    if (isLog_) std::cout << "# element is a collection! " << std::endl;
                    //dictStack.push(parseDict(dictStack.top(), key));
                    dictStack[indent] = parseDict(dictStack[indent - 1], key); // putting new dict with key into currently opened dict, and adding to stack
                    //currentIndex = indent;
                    if (isLog_) std::cout << "# addded dict at indent - " << indent << std::endl;

                } else {
                    if (isLog_) std::cout << "# element is a part of a dict! " << std::endl;
                    parseDictElement(dictStack[indent - 1], key, value);
                    if (isLog_) std::cout << "# added element to dict at indent - " << indent - 1 << std::endl;
                    /*
                    if (indent > 0) {
                        if (isLog_) std::cout << "# element is a part of a dict! " << std::endl;
                        parseDictElement(dictStack.top(), key, value);
                    } else {
                        if (isLog_) std::cout << "# element is a value! " << std::endl;
                        parseElement(dictStack.top(), key, value);
                    }
                    */
                }
            }

            // currentIndent = indent;
        }
    }
    void save(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            return;
        }

        file << getString();

        file.close();
    }

private:
    bool isLog_;
    std::map<std::string, std::any> data_;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    std::string getType(const std::string& value) {
        if (value.empty()) {
            return "";
        } else if (value.front() == '[' && value.back() == ']') {
            // Splitting elements of the array, trimming each, and converting to `std::any`
            std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
            std::string item;
            std::string vectorType = "";
            while (getline(ss, item, ',')) {
                vectorType = getType(trim(item));
                break;
            }
            return "vector_"+vectorType;
        } else if (std::all_of(value.begin(), value.end(), ::isdigit)) {
            return "int";
        } else if (value.find('.') != std::string::npos) {
            return "float";
        } else if (value == "true") {
            return "bool";
        } else if (value == "false") {
            return "bool";
        } else {
            return "string";
        }
    }
    bool startswith(const std::string& str, const std::string& prefix) {
        if (str.size() < prefix.size()) {
            return false;
        }
        return str.compare(0, prefix.size(), prefix) == 0;
    }
    std::any parseValue(const std::string& value) {
        std::string type = getType(value);
        if (isLog_) {
            std::cout << "parsing value - " << value << ", with type - " << type << std::endl;
        }
        if (value.empty()) {
            return {};
        } else if (startswith(type, "vector_")) {
            if (type == "vector_float") {
                std::vector<float> vecFloat;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    vecFloat.push_back(std::stof(trim(item)));
                }
                return vecFloat;
            } else if (type == "vector_int") {
                std::vector<int> vecInt;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    vecInt.push_back(std::stoi(trim(item)));
                }
                return vecInt;
            } else if (type == "vector_bool") {
                std::vector<float> vecBool;
                std::stringstream ss(value.substr(1, value.size() - 2)); // Remove the brackets
                std::string item;
                while (getline(ss, item, ',')) {
                    if (trim(item) == "true") {
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
                    std::string itemS = trim(item);
                    itemS.erase(std::remove(itemS.begin(), itemS.end(), '"'), itemS.end());
                    if (isLog_) {
                        std::cout << "parsing value - " << itemS << std::endl;
                    }
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
            } else {
                return value;
            }
        }

    }
    void parseDictElement(Dict* dict, std::string& key, std::string& value) {
        //std::cout << "## setting element of dict " << std::endl;
        dict->set(key, parseValue(value));
        if (key == "is_log") {
            isLog_ = (value == "true");
        }
    }
    Dict* parseDict(Dict* dict, std::string& collectionName) {
        dict->set(collectionName, Dict());
        return dict->get<Dict>(collectionName);
    }
    void parseListElement(std::string& collectionName, std::string& value) {
        nevc::List* list = get<nevc::List>(collectionName);
        if (list) {
            // If the list exists, add the parsed value to it
            list->add(parseValue(value));
        } else {
            // If the list does not exist, create a new List and add the parsed value
            nevc::List newList;
            newList.add(parseValue(value));
            // Store the new List in the dictionary
            data_[collectionName] = newList;
        }
    }
};

}

#endif // DICT_H