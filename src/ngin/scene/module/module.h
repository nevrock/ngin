#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
#include <algorithm>

namespace ngin {
namespace scene {

enum class ModuleEditKind {
    EditParent,
    AddChild,
    RemoveChild,
    AddModule,
    RemoveModule
};

struct ModuleEdit {
    unsigned int object_id;
    unsigned int new_value;
    ModuleEditKind kind;
};

class Module {
public:
    Module(const unsigned int id, const std::string& name) : id_(id), name_(name) {} // Initialize parent_ here

    unsigned int& get_id() {
        return id_;
    }
    const std::string& get_name() const {
        return name_;
    }

    void set_level(unsigned int level) {
        level_ = level;
    }
    unsigned int& get_level() {
        return level_;
    }

    virtual void from_atlas(Atlas* data) = 0;

private:
    unsigned int id_;
    std::string name_;

    unsigned int level_;
};

}
}

#endif // MODULE_H