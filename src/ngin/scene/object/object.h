#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include <algorithm>

namespace ngin {
namespace scene {

enum ObjectEditKind {
    EditParent,
    AddChild,
    RemoveChild,
    AddModule,
    RemoveModule
};

struct ObjectEdit {
    unsigned int object_id;
    unsigned int new_value;
    ObjectEditKind kind;
};

class Object {
public:
    Object(const unsigned int id, const std::string& name) : id_(id), name_(name), parent_(0) {} // Initialize parent_ here

    unsigned int& get_id() {
        return id_;
    }
    unsigned int& get_parent() {
        return parent_;
    }
    const std::string& get_name() const {
        return name_;
    }
    unsigned int& get_level() {
        return level_;
    }

    void set_parent(unsigned int parent) {
        parent_ = parent;
    }
    void set_level(unsigned int level) {
        level_ = level;
    }

    void add_child(unsigned int child) {
        children_.push_back(child);
    }
    void remove_child(unsigned int child) {
        children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
    }

    void add_module(unsigned int module) {
        modules_.push_back(module);
    }
    void remove_module(unsigned int module) {
        modules_.erase(std::remove(modules_.begin(), modules_.end(), module), modules_.end());
    }

    std::vector<unsigned int>& get_children() {
        return children_;
    }
    std::vector<unsigned int>& get_modules() {
        return modules_;
    }

private:
    unsigned int id_;
    std::string name_;
    unsigned int level_;

    unsigned int parent_;
    std::vector<unsigned int> children_;
    std::vector<unsigned int> modules_;

};

}
}

#endif // OBJECT_H