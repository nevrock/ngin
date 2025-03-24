#ifndef NODE_PORT_H
#define NODE_PORT_H

#include <ngin/lex.h>
#include <ngin/log.h>

class NodePort {
public:
    NodePort(unsigned int id, const std::string& type = "default") : id_(id), type_(type) {
        Log::console("NodePort created with ID: " + std::to_string(id_) + " and type: " + type_, 1);
    }
    unsigned int getId() const { return id_; }
    Lex getData() { return data_; }
    const std::string& getType() { return type_; }
    void transferDataFrom(NodePort* other) {
        if (other) {
            data_ = other->getData();

            //other->clearData(); 
        }
    }

    template<typename T>
    T getVar(const std::string& key, const T& defaultValue) const {
        return data_.getC<T>(key, defaultValue);
    }
    void setVar(const std::string& key, const std::any& value) {
        data_.set(key, value);
    }
    bool containsVar(const std::string& key) const {
        return data_.contains(key);
    }
    void clearData() {
        data_.clear();
    }

private:
    unsigned int id_;
    std::string type_;
    Lex data_;
};

#endif // NODE_PORT_H
