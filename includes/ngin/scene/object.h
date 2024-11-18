#ifndef OBJECT_H
#define OBJECT_H

#include <ngin/scene/i_object.h>
#include <ngin/scene/component.h>
#include <ngin/scene/point_ui.h>
#include <ngin/log.h>
#include <ngin/utils/fileutils.h>
#include <ngin/collections/nevf.h> 

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <queue>

class Object : public IObject {
public:
    // Constructors
    Object() : isParent(false), point(std::make_unique<Point>()) {}

    Object(const Nevf& dict, unsigned int hIndex, 
        IObject* parentObj = nullptr, bool isParentObj = false) : isParent(isParentObj), 
        point(std::make_unique<Point>()), parent(parentObj), hierarchyIndex(hIndex) {
        loadFromNevf(dict);
    }

    ~Object() {
        // Log the destruction of the object if necessary
        Log::console("Destroying Object: " + name);
    }

    // Getters
    Point& getPoint() override {
        return *point;
    }
    const Point& getPoint() const override {
        return *point;
    }
    PointUi& getPointUi() override {
        PointUi* uiPoint = dynamic_cast<PointUi*>(point.get());
        if (!uiPoint) {
            throw std::runtime_error("Current Point instance is not of type PointUi");
        }
        return *uiPoint;
    }
    const PointUi& getPointUi() const override {
        const PointUi* uiPoint = dynamic_cast<const PointUi*>(point.get());
        if (!uiPoint) {
            throw std::runtime_error("Current Point instance is not of type PointUi");
        }
        return *uiPoint;
    }    
    std::string getName() const override {
        return name;
    }
    IObject* getParent() const override {
        return parent;
    }

    // Setters
    void setPoint(const Point& newPoint) {
        *point = newPoint;
    }

    // Load from JSON
    void loadFromNevf(const Nevf& dConst) {
        Nevf d = dConst;  // Create a modifiable copy of dConst
        isUiPoint = false;
        if (d.contains("inherit")) {
            std::string inheritFromObj  = d.getC<std::string>("inherit", "");
            Nevf dOther;
            dOther.read(FileUtils::getResourcePath("nevf/objects/" + inheritFromObj + ".nevf"));
            d.sync(&dOther);
            Log::console("object inherit from: " + inheritFromObj);
            d.print();
        }
        if (d.contains("name")) {
            name = d.getC<std::string>("name", "object"); 
        }
        if (parent != nullptr) {
            Log::console(":::OBJECT::: " + getName() + ", " + std::to_string(hierarchyIndex) + 
                ", with parent - " + parent->getName());
        }
        if (d.contains("point")) {
            Nevf pD;
            loadPoint(d.getC<Nevf>("point", pD));
        } else {
            point->setScale(glm::vec3(1.0,1.0,1.0));
        }
        if (d.contains("components")) {
            Nevf cD;
            loadComponents(d.getC<Nevf>("components", cD));
        }
        if (d.contains("children")) {
            Nevf chD;
            loadChildren(d.getC<Nevf>("children", chD));
        }
    }
    void loadChildren(const Nevf& d) {
        for (auto& childNevf : d.data()) {
            std::string name = std::any_cast<std::string>(childNevf.first);
            Nevf childData;
            childData = d.getC<Nevf>(name, childData);
            
            std::shared_ptr<Object> child = std::make_shared<Object>(childData, hierarchyIndex+1, this, true);
            children[name] = child;
        }
    }
    void addChild(const std::string& key, const Nevf& args) {
        if (args.contains(key)) {
            Nevf childNevf;
            childNevf = args.getC<Nevf>(key, childNevf);
            if (childNevf.contains("name")) {
                std::string childName = childNevf.getC<std::string>("name", "");
                if (childName == "") {
                    return;
                }
                std::shared_ptr<Object> child = std::make_shared<Object>(childNevf, hierarchyIndex+1, this, true);
                children[childName] = child;
                Log::console("Added child with key '" + key + "': " + childName);
            } else {
                Log::console("Nevf under key '" + key + "' does not contain a 'name' key, child not added.");
            }
        } else {
            Log::console("Nevf does not contain key: " + key);
        }
    }
    void removeChild(const std::string& name) {
        auto it = children.find(name);
        if (it != children.end()) {
            children.erase(it);
            Log::console("Removed child: " + name);
        } else {
            Log::console("Child not found: " + name);
        }
    }


    // Update (now a normal member function)
    void launch() {
        // Log::console("object update, with name: " + name);  
        for (auto& component : components) {
            component->launch();  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->launch();  // Recursively update child objects
        }
    }
    void updateAnimation(float dt) {
        // Log::console("object update, with name: " + name);  
        for (auto& component : components) {
            component->updateAnimation(dt);  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->updateAnimation(dt);  // Recursively update child objects
        }
    }
    void update() {
        // Log::console("object update, with name: " + name);  
        for (auto& component : components) {
            component->update();  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->update();  // Recursively update child objects
        }
    }
    void updatePhysics(float dt) {
        for (auto& component : components) {
            component->updatePhysics(dt);  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->updatePhysics(dt);  // Recursively update child objects
        }
    }
    void updatePreRender(const unsigned int index, Shader& shader) {
        for (auto& component : components) {
            component->updatePreRender(index, shader);  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->updatePreRender(index, shader);  // Recursively update child objects
        }
    }

    void updateRender(const unsigned int index, Shader& shader) {
        for (auto& component : components) {
            component->updateRender(index, shader);  // Call the update method on each component
        }
        for (auto& [childName, child] : children) {
            child->updateRender(index, shader);  // Recursively update child objects
        }
    }

    // point extensions:
    glm::mat4 getWorldMatrix(bool includeSelf = true) const override {
        //Log::console("object get world matrix! " + name);
        glm::mat4 model = point->getModelMatrix();
        if (!includeSelf) {
            model = glm::mat4(1.0);
        }
        if (hasParent()) {
            if ((isUiPoint && parent->isUi()) || !isUiPoint)
                model *= parent->getWorldMatrix();
        }
        return model;
    }
    glm::vec3 getWorldPosition() const override {
        glm::vec3 p = getPoint().getPosition();
        if (hasParent()) {
            if ((isUiPoint && parent->isUi()) || !isUiPoint)
                p += parent->getWorldPosition();
        }
        return p;
    }

    // Component management
    template<typename T>
    std::shared_ptr<T> getComponent() const {
        for (const auto& component : components) {
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(component);
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }

    template<typename T>
    std::vector<std::shared_ptr<T>> getComponents() const {
        std::vector<std::shared_ptr<T>> matchingComponents;
        for (const auto& component : components) {
            std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(component);
            if (casted) {
                matchingComponents.push_back(casted);
            }
        }
        return matchingComponents;
    }

    template<typename T>
    std::vector<std::shared_ptr<T>> getComponentsInChildren() const {
        std::vector<std::shared_ptr<T>> result;
        std::queue<std::shared_ptr<Object>> queue;

        // Enqueue all direct children of this object
        for (const auto& [childName, child] : children) {
            queue.push(child);
        }

        while (!queue.empty()) {
            std::shared_ptr<Object> current = queue.front();
            queue.pop();

            // Collect components of the current object
            std::vector<std::shared_ptr<T>> currentComponents = current->getComponents<T>();
            result.insert(result.end(), currentComponents.begin(), currentComponents.end());

            // Enqueue all children of the current object
            for (const auto& [childName, child] : current->children) {
                queue.push(child);
            }
        }

        return result;
    }

    template<typename T>
    void getAllComponents(std::vector<std::shared_ptr<T>>*& outComponentsPtr = nullptr) {
        if (!outComponentsPtr) {
            outComponentsPtr = new std::vector<std::shared_ptr<T>>();
        }

        // Get components of the current object
        std::vector<std::shared_ptr<T>> myComponents = getComponents<T>();
        outComponentsPtr->insert(outComponentsPtr->end(), myComponents.begin(), myComponents.end());

        // Recursively get components from all children
        for (const auto& [childName, child] : children) {
            child->getAllComponents(outComponentsPtr);
        }
    }

    using ComponentFactory = std::function<std::shared_ptr<Component>(Object*)>;

    static std::unordered_map<std::string, ComponentFactory>& getComponentFactories() {
        static std::unordered_map<std::string, Object::ComponentFactory> componentFactories;
        return componentFactories;
    }
    static void registerComponent(const std::string& name, ComponentFactory factory) {
        getComponentFactories()[name] = factory;
    }

    bool hasParent() const override {
        return isParent;
    }
    bool isUi() override {
        return isUiPoint;
    }
    unsigned int getHierarchyIndex() const override {
        return hierarchyIndex;
    }

private:
    IObject* parent;
    std::unique_ptr<Point> point;
    std::vector<std::shared_ptr<Component>> components;
    std::unordered_map<std::string, std::shared_ptr<Object>> children;
    const bool isParent;
    bool isUiPoint;
    std::string name;

    void resetPoint(bool isUiPoint) {
        point.reset();  // Destroy the current Point object
        if (isUiPoint) {
            point = std::make_unique<PointUi>();  // Create a new Point object
            Log::console("object reset point, set to ui!");
        } else {
            point = std::make_unique<Point>();  // Create a new Point object
        }
    }

    void loadPoint(const Nevf& d) {
        Log::console("object load point!");
        if (d.contains("is_ui") && d.getC<bool>("is_ui", true)) {
            resetPoint(true);
            isUiPoint = true;
        }
        if (d.contains("position") && d.isType("position", typeid(std::vector<float>))) {
            point->setPosition(d.getVec("position", glm::vec3(0.0)));
        }
        if (d.contains("rotation") && d.isType("rotation", typeid(std::vector<float>))) {
            point->setRotation(d.getVec("rotation", glm::vec3(0.0)));
        }
        if (d.contains("scale") && d.isType("scale", typeid(std::vector<float>))) {
            point->setScale(d.getVec("scale", glm::vec3(0.0)));
        }
        if (d.contains("size") && d.isType("size", typeid(std::vector<float>))) {
            getPointUi().setSize(d.getVec2("size", glm::vec2(0.0)));
        }
    }


protected:
    unsigned int hierarchyIndex;
    void loadComponents(const Nevf& dict, bool isLaunch = false) {
        for (auto& element : dict.data()) {
            std::string type = element.first;
            Nevf componentData;
            componentData = dict.getC<Nevf>(type, componentData);
            
            auto it = getComponentFactories().find(type);
            if (it != getComponentFactories().end()) {
                std::shared_ptr<Component> component = it->second(this);
                component->loadFromNevf(componentData);
                components.push_back(component);

                if (isLaunch)
                    component->launch();
            } else {
                std::cerr << "Unknown component type: " << type << std::endl;
            }
        }
    }
};

#endif // OBJECT_H
