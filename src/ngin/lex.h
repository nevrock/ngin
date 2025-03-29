#ifndef LEX_H
#define LEX_H

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

#include <glm/glm.hpp>
#include <unordered_map>

class Lex {
public:
    Lex() = default;
    ~Lex() = default;
    Lex(const Lex& other) {
        data_ = other.data_; // Use the map's copy assignment
        name_ = other.getName();
    }
    Lex& operator=(const Lex& other) {
        if (this != &other) {
            data_ = other.data_; // Use the map's copy assignment
            name_ = other.getName();
        }
        return *this;
    }
    Lex(Lex&& other) noexcept {
        data_ = std::move(other.data_);
        name_ = other.getName();
    }
    Lex& operator=(Lex&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }

    void set(const std::string& key, std::any value) {
        if (data_.find(key) == data_.end()) {
            keyOrder_.push_back(key); // Track insertion order
        }
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
        // //std::cout << "dict search - failed with key " << key << std::endl;
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
            //std::cout << "dict failed to convert to vec - list length mismatch" << std::endl;
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
                //std::cout << "dict failed to convert to vec - bad any cast" << std::endl;
                return defaultValue; // Return default if the index is out of range
            } catch (const std::out_of_range& e2) {
                //std::cout << "dict failed to convert to vec - out of range" << std::endl;
                return defaultValue; // Return default if the index is out of range
            }
            return vec; // Return default if there's a type mismatch
        } catch (const std::out_of_range& e) {
            //std::cout << "dict failed to convert to vec - out of range" << std::endl;
            return defaultValue; // Return default if the index is out of range
        }
        return vec;
    }
    glm::vec2 getVec2(const std::string& key, const glm::vec2& defaultValue) const {
        std::vector<float> l = getC<std::vector<float>>(key, std::vector<float>{0.0,0.0});
        if (l.size() != 2) {
            //std::cout << "dict failed to convert to vec - list length mismatch" << std::endl;
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
                //std::cout << "dict failed to convert to vec - bad any cast" << std::endl;
                return defaultValue; // Return default if the index is out of range
            } catch (const std::out_of_range& e2) {
                //std::cout << "dict failed to convert to vec - out of range" << std::endl;
                return defaultValue; // Return default if the index is out of range
            }
            return vec; // Return default if there's a type mismatch
        } catch (const std::out_of_range& e) {
            //std::cout << "dict failed to convert to vec - out of range" << std::endl;
            return defaultValue; // Return default if the index is out of range
        }
        return vec;
    }
    glm::vec4 getVec4(const std::string& key, const glm::vec4& defaultValue) const {
        std::vector<float> l = getC<std::vector<float>>(key, std::vector<float>{0.0, 0.0, 0.0, 0.0});
        if (l.size() != 4) {
            //std::cout << "dict failed to convert to vec - list length mismatch" << std::endl;
            return defaultValue; // Return default if the length is not exactly 4
        }
        glm::vec4 vec(0.0f);
        try {
            for (int i = 0; i < 4; ++i) {
                vec[i] = l[i];
            }
        } catch (const std::bad_any_cast& e) {
            try {
                std::vector<int> li = getC<std::vector<int>>(key, std::vector<int>{0, 0, 0, 0});
                for (int i = 0; i < 4; ++i) {
                    vec[i] = li[i];
                }
            } catch (const std::bad_any_cast& e2) {
                //std::cout << "dict failed to convert to vec - bad any cast" << std::endl;
                return defaultValue; // Return default if the index is out of range
            } catch (const std::out_of_range& e2) {
                //std::cout << "dict failed to convert to vec - out of range" << std::endl;
                return defaultValue; // Return default if the index is out of range
            }
            return vec; // Return default if there's a type mismatch
        } catch (const std::out_of_range& e) {
            //std::cout << "dict failed to convert to vec - out of range" << std::endl;
            return defaultValue; // Return default if the index is out of range
        }
        return vec;
    }
    std::string getString(int indent = 0) const {
        std::string result;
        for (const auto& pair : data_) {
            result += std::string(indent, ' ') + pair.first + ": ";
            if (pair.second.type() == typeid(Lex)) {
                result += "\n";
                result += std::any_cast<const Lex&>(pair.second).getString(indent + 4);
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
    std::string getTypeForKey(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
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
        } else if (type == typeid(Lex)) {
            return "nevf";
        } else {
            return "unknown"; // Return "unknown" for any other type
        }
    }
    bool contains(const std::string& key) const {
        return data_.find(key) != data_.end();
    }
    bool has(const std::string& key) const {
        return contains(key);
    }

    const std::unordered_map<std::string, std::any>& data() const { return data_; }
    void sync(const Lex* other, bool overwrite = false) {
        if (other == nullptr) {
            std::cerr << "provided dictionary pointer is null" << std::endl;
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
            keyOrder_.erase(std::remove(keyOrder_.begin(), keyOrder_.end(), key), keyOrder_.end());
        }
    }
    void print(int indent = 0) const {
        std::cout << "## lexicon <" << name_ << "> print out! ##" << std::endl;
        std::cout << getString(indent) << std::endl;
    }
    void read(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        std::unordered_map<int, Lex*> dictStack;
        dictStack[-1] = this;


        //if (isLog_) std::cout << "### dict parsing ### - " << filename << std::endl;

        std::string collectionName = std::string("");

        for (std::string line; getline(file, line); ) {
            if (line.empty() || (line[0] == '/' && line[1] == '/')) continue;

            //if (isLog_) std::cout << line << std::endl;
            
            int indent = line.find_first_not_of(' ');
            indent = indent / 4;

            //if (isLog_) std::cout << "# indent - " << indent << std::endl;

            line = trim(line);
            int delimiterPos = line.find(':');
            if (delimiterPos == std::string::npos) {
                continue;
            } else {                
                std::string key = line.substr(0, delimiterPos);
                std::string value = delimiterPos < line.size() - 1 ? trim(line.substr(delimiterPos + 1)) : "";
                if (value == "") {
                    // could be a list of a dict
                    //if (isLog_) std::cout << "# element is a list or collection! " << std::endl;
                    dictStack[indent] = parseLex(dictStack[indent - 1], key); // putting new dict with key into currently opened dict, and adding to stack
                    //if (isLog_) std::cout << "# addded dict at indent - " << indent << std::endl;

                } else {
                    //if (isLog_) std::cout << "# element is a part of a dict! " << std::endl;
                    parseLexElement(dictStack[indent - 1], key, value);
                    //if (isLog_) std::cout << "# added element to dict at indent - " << indent - 1 << std::endl;
                }
            }

            // currentIndent = indent;
        }
    }
    void save(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "failed to open file for writing: " << filepath << std::endl;
            return;
        }

        file << getString();

        file.close();
    }

    void clear() {
        data_.clear();
        keyOrder_.clear();
    }

    size_t length() const {
        return data_.size();
    }

    auto begin() {
        return data_.begin();
    }
    auto end() {
        return data_.end();
    }
    auto begin() const {
        return data_.begin();
    }
    auto end() const {
        return data_.end();
    }
    // const auto& key : dict.keys()
    std::vector<std::string> keys() const {
        return keyOrder_; // Return keys in insertion order
    }
    // Iterable object for keys (optional advanced version)
    class KeyIterable {
    public:
        explicit KeyIterable(const std::unordered_map<std::string, std::any>& data) : data_(data) {}

        auto begin() const { return data_.cbegin(); }
        auto end() const { return data_.cend(); }

    private:
        const std::unordered_map<std::string, std::any>& data_;
    };

    // Method to provide iterable access to keys
    KeyIterable keyIterable() const {
        return KeyIterable(data_);
    }


    std::any& operator[](const std::string& key) {
        return data_[key];
    }

    // Const operator[] for read-only access
    const std::any& operator[](const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        throw std::out_of_range("Key not found: " + key);
    }


    void setLog(bool log) {
        isLog_ = log;
    }
    void setName(const std::string& name) {
        name_ = name;
    }
    std::string getName() const {
        return name_;
    }


private:
    bool isLog_ = false;
    std::string name_;
    std::unordered_map<std::string, std::any> data_;
    std::vector<std::string> keyOrder_; // Stores keys in insertion order

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
        } else if (value.front() == '"' && value.back() == '"') {
            return "string";
        } else if (value.front() == '\'' && value.back() == '\'') {
            return "string";
        } else if (std::all_of(value.begin(), value.end(), [](char c) { return ::isdigit(c) || c == '-'; }) && 
                   (value.front() == '-' ? value.size() > 1 : true)) {
            return "int";
        } else if ((value.front() == '-' && std::count_if(value.begin() + 1, value.end(), [](char c) { return ::isdigit(c) || c == '.'; }) == value.size() - 1 && std::count(value.begin() + 1, value.end(), '.') <= 1) ||
                   (std::count_if(value.begin(), value.end(), [](char c) { return ::isdigit(c) || c == '.'; }) == value.size() && std::count(value.begin(), value.end(), '.') <= 1)) {
            return "float";
        } else if (value == "true" || value == "false") {
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
            //std::cout << "parsing value - " << value << ", with type - " << type << std::endl;
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
                std::vector<bool> vecBool;
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
                    itemS.erase(std::remove(itemS.begin(), itemS.end(), '\''), itemS.end());
                    if (isLog_) {
                        //std::cout << "parsing value - " << itemS << std::endl;
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
            } else if (value.front() == '\'' && value.back() == '\'') {
                return value.substr(1, value.length() - 2); 
            } else {
                return value;
            }
        }
        return {}; // Add a default return value to avoid warnings
    }
    void parseLexElement(Lex* dict, std::string& key, std::string& value) {
        //std::cout << "## setting element of dict " << std::endl;
        dict->set(key, parseValue(value));
        if (key == "is_log") {
            dict->setLog(value == "true");
        } else if (key == "name") {
            //std::cout << "## setting name of dict " << value << std::endl;
            dict->setName(value);
        }
    }
    Lex* parseLex(Lex* dict, std::string& collectionName) {
        dict->set(collectionName, Lex());
        return dict->get<Lex>(collectionName);
    }
};

#endif // LEX_H