#ifndef OBJECT_H
#define OBJECT_H

#include <ngin/scene/i_component.h>
#include <ngin/log.h>
#include <memory> // Include memory header for std::enable_shared_from_this
#include <functional>
#include <unordered_map>
#include <string>

class Object : public IObject {
public:

    Object(std::string name, Lex lex) 
        : IObject(name, lex), parent_(nullptr) {
    }
    Object(std::string name, Lex lex, Object* parent)
        : IObject(name, lex), parent_(parent) {
    }

    void init() override {
        componentInit();
    }
    void launch() override {
        componentLaunch();
        for (auto& [name, child] : children_) {
            child->launch();
        }
    }
    void updateLogic() override {
        componentUpdateLogic();
        for (auto& [name, child] : children_) {
            child->updateLogic();
        }
    }
    void updateTransform() override {
        // cook transform position 
        if (parent_) {
            transform_->setParentModel(parent_->getTransform()->getWorldModelMatrix());
        } else {
            transform_->setParentModel(glm::mat4(1.0f));
        }

        transform_->execute();

        for (auto& [name, child] : children_) {
            child->updateTransform();
        }

        //Log::console("object: " + name_ + ", update transformm, with position: " + std::to_string(transform_->getWorldPosition().x) + ", " + std::to_string(transform_->getWorldPosition().y) + ", " + std::to_string(transform_->getWorldPosition().z));  
    }

    void build() {
        createChildren();

        buildTransform();
        buildComponents();

        init();

        buildChildren();
    }

    void componentInit() {
        for (auto& component : components_) {
            component->init();
        }
    }
    void componentLaunch() {
        for (auto& component : components_) {
            component->launch();
        }
    }
    void componentUpdateLogic() {
        for (auto& component : components_) {
            component->update();
        }
    }
    void componentUpdateLate() {
        for (auto& component : components_) {
            component->updateLate();
        }
    }

    template<typename T>
    std::vector<T*> getComponents() {
        std::vector<T*> components;
        for (auto& component : components_) {
            auto casted = dynamic_cast<T*>(component.get());
            if (casted) {
                components.push_back(casted);
            }
        }
        return components;
    }
    template<typename T>
    T* getComponent(const std::string& name) {
        for (auto& component : components_) {
            if (component->getName() == name) {
                return dynamic_cast<T*>(component.get());
            }
        }
        return nullptr;
    }
    template<typename T>
    T* getComponent() {
        for (auto& component : components_) {
            auto casted = dynamic_cast<T*>(component.get());
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }

    void newComponent(const std::string type, const std::string name, const Lex& lex) {
        auto it = getComponentFactories().find(type);
        if (it != getComponentFactories().end()) {
            std::unique_ptr<IComponent> component = it->second(name, lex, this);
            components_.push_back(std::move(component));
        } else {
            std::cerr << "unknown component type: " << type << std::endl;
        }
    }
    template<typename T>
    void addComponent(std::unique_ptr<T> component) {
        static_assert(std::is_base_of<IComponent, T>::value, "type T must be derived from IComponent");
        components_.push_back(std::move(component));
    }

    Object* getParent() { return parent_; }  
    std::unique_ptr<IObject> getChild(const std::string& name) {
        auto it = children_.find(name);
        if (it != children_.end()) {
            return std::move(it->second);
        }
        return nullptr;
    }   

    // component
    using ComponentFactory = std::function<std::unique_ptr<IComponent>(const std::string name, const Lex& lex, IObject* parent)>;
    static std::unordered_map<std::string, ComponentFactory>& getComponentFactories() {
        static std::unordered_map<std::string, Object::ComponentFactory> componentFactories;
        return componentFactories;
    }
    static void registerComponent(const std::string& name, ComponentFactory factory) {
        getComponentFactories()[name] = factory;
    }

    void log(int indent = 0) const {
        std::string indentation(indent, ' ');
        Log::console(indentation + "Object: " + getName(), 1); 
        for (auto& [name, child] : children_) {
            child->log(indent + 4);
        }
    }

    void registerComponent(const std::string& name, std::function<void()> factory);

protected:
    std::vector<std::unique_ptr<IComponent>> components_;

    std::map<std::string, std::unique_ptr<Object>> children_;
    Object* parent_;

    void createChildren() {
        auto children = lex_.getC<Lex>("children", Lex());
        for (const auto& [name, child] : children.data()) {
            children_[name] = std::make_unique<Object>(name, std::any_cast<Lex>(child), this);
        }
    }
    virtual void buildComponents() {
        auto components = lex_.getC<Lex>("components", Lex());
        for (const auto& [name, component] : components.data()) {
            Lex childLex = std::any_cast<Lex>(component);
            std::string type = childLex.getC<std::string>("type", "");
            newComponent(type, name, std::any_cast<Lex>(component));
        }

    }
    void buildChildren() {
        for (auto& [name, child] : children_) {
            child->build();
        }
    }

private:
    std::unordered_map<std::string, std::function<void()>> componentFactories;

};

#endif // OBJECT_H
