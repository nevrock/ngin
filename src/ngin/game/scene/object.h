#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <memory>

#include <ngin/debug/logger.h>
#include <ngin/atlas/atlas.h>
#include <ngin/game/scene/module.h>

class Object {
public:
    Object() = default;
    Object(std::string name, Atlas& data) : name_(name) {
        logger_ = new Logger("Object::" + name);
    }
    ~Object() {
        for (auto& child : children_) {
            delete child.second;
        }
        for (auto& module : modules_) {
            delete module;
        }
    }

    void setup_from_atlas(Atlas& data) {

    }

    virtual void build() {}

    // Initialization and launch methods
    virtual void init() {}
    virtual void launch() {}

    // Update methods in order
    virtual void update_logic() {} // Perform any movement related logic here
    virtual void update_transform(glm::mat4 parentModel = glm::mat4(1.0f)) {}
    virtual void update_late() {} // Transform is fixed at this stage

    const std::string& get_name() const {
        return name_;
    }

    template<typename T>
    std::vector<T*> get_modules() {
        std::vector<T*> modules;
        for (auto& component : modules_) {
            modules.push_back(component);
        }
        return modules;
    }
    template<typename T>
    T* get_module() {
        for (auto& component : modules_) {
            auto casted = component;
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }
    void add_module(Module* module) {
        modules_.push_back(module);
    }
    void new_module(std::string name, std::string kind, Atlas* data) {
        auto it = get_module_factories().find(kind);
        if (it != get_module_factories().end()) {
            Module* module = it->second(name, *this, *data).release();
            add_module(module);
        } else {
            logger_->error("Module not found of kind: " + kind);
        }
    }
    Object* search_children(const std::string& name, bool recursive = false) {
        for (const auto& [childName, child] : children_) {
            if (child->get_name() == name) {
                return child;
            }
            if (recursive) {
                Object* found = child->search_children(name, true);
                if (found) {
                    return found;
                }
            }
        }
        return nullptr;
    }

    // --- --- MODULES --- ---
    using ModuleFactory = std::function<std::unique_ptr<Module>(const std::string name, Object& parent, Atlas& data)>;
    static std::unordered_map<std::string, ModuleFactory>& get_module_factories() {
        static std::unordered_map<std::string, Object::ModuleFactory> module_factories;
        return module_factories;
    }
    static void register_module(const std::string& name, ModuleFactory factory) {
        get_module_factories()[name] = factory;
    }

protected:
    Logger* logger_;
    std::map<std::string, Object*> children_;
    std::vector<Module*> modules_;
    std::string name_;

    virtual void build_modules(Atlas& data) {
        Atlas* modules = new Atlas();
        modules = data.get<Atlas>("modules", modules);
        for (const auto& name : modules->keys()) {
            Atlas* module = new Atlas();
            module = modules->get<Atlas>(name, module);
            std::string module_kind = "";
            std::string* kind = module->get<std::string>("kind", &module_kind);

            new_module(name, *kind, module);
        }

    }
};

#endif // OBJECT_H
