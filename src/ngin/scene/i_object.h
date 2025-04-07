#ifndef I_OBJECT_H
#define I_OBJECT_H

#include <map>
#include <memory>

#include <ngin/lex.h>
#include <ngin/data/point_data.h>
#include <ngin/data/rect_data.h>
#include <ngin/log.h>

class IComponent;

class IObject {
public:
    IObject() = default;
    IObject(std::string name, Lex lex) : name_(name), lex_(lex) {
    }

    virtual void init() {}
    virtual void launch() {}
    virtual void build() {}
    virtual void updateLogic() {}
    virtual void log(int indent = 0) const {}
    virtual void updateTransform(glm::mat4 parentModel = glm::mat4(1.0f)) {}

    const std::string& getName() const {
        return name_;
    }

    TransformData* getTransform() {
        return transform_.get();
    }
    PointData* getPointTransform() {
        return static_cast<PointData*>(transform_.get());
    }
    RectData* getRectTransform() {
        return static_cast<RectData*>(transform_.get());
    }

    std::vector<PointData*> getChildPointTransforms() {
        std::vector<PointData*> points;
        for (const auto& [name, child] : children_) {
            PointData* point = child->getPointTransform();
            if (point) {
                points.push_back(point);
            }
        }
        return points;
    }
    std::vector<RectData*> getChildRectTransforms() {
        std::vector<RectData*> rects;
        for (const auto& [name, child] : children_) {
            RectData* rect = child->getRectTransform();
            if (rect) {
                rects.push_back(rect);
            }
        }
        return rects;
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
    T* getComponent() {
        for (auto& component : components_) {
            auto casted = dynamic_cast<T*>(component.get());
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }

protected:
    std::unique_ptr<TransformData> transform_;
    std::map<std::string, std::unique_ptr<IObject>> children_;
    std::vector<std::unique_ptr<IComponent>> components_;

    void buildTransform() {
        if (lex_.contains("point")) {
            transform_ = std::make_unique<PointData>(lex_.getC<Lex>("point", Lex()));
        } else if (lex_.contains("rect")) {
            transform_ = std::make_unique<RectData>(lex_.getC<Lex>("rect", Lex()));
        } else {
            transform_ = std::make_unique<PointData>(Lex());
        }
    }

    std::string name_;
    Lex lex_;
};

#endif // I_OBJECT_H
