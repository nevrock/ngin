#ifndef I_OBJECT_H
#define I_OBJECT_H

#include <map>
#include <memory>

#include <ngin/lex.h>
#include <ngin/data/point_data.h>
#include <ngin/data/rect_data.h>
#include <ngin/log.h>

class IObject {
public:
    IObject() = default;
    IObject(std::string name, Lex lex) : name_(name), lex_(lex) {
    }

    virtual void init() {}
    virtual void launch() {}
    virtual void updateLogic() {}
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

protected:
    std::unique_ptr<TransformData> transform_;

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
