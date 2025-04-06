#ifndef I_COMPONENT_H
#define I_COMPONENT_H

#include <memory>
#include <ngin/scene/i_object.h>
#include <ngin/lex.h>

class IComponent {
public:
    IComponent() = default;
    IComponent(const std::string name, const Lex& lex, IObject* parent) 
        : name_(name), lex_(lex), parent_(parent) {}

    std::string getName() const { return name_; }

    virtual void init() = 0;
    virtual void launch() = 0;
    virtual void update() = 0;
    virtual void updateLate() = 0;

    TransformData* getTransform() {
        return parent_->getTransform();
    }
    PointData* getPointTransform() {
        return parent_->getPointTransform();
    }
    RectData* getRectTransform() const {
        return parent_->getRectTransform();
    }

    bool raycast(glm::vec2 pos) {
        RectData* rect = getRectTransform();
        if (rect) {
            return rect->raycast(pos);
        } else {
            Log::console(pos, "raycast failed: no rect transform", 1);
            return false;
        }
    }

protected:
    IObject* parent_;
    std::string name_;
    Lex lex_;
};

#endif // COMPONENT_H
