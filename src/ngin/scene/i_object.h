#ifndef I_OBJECT_H
#define I_OBJECT_H

#include <map>
#include <memory>

#include <ngin/lex.h>
#include <ngin/data/transform_data.h>
#include <ngin/log.h>

class IObject {
public:
    IObject() = default;
    IObject(std::string name, Lex lex) : name_(name), lex_(lex) {
    }

    virtual void init() {}
    virtual void launch() {}
    virtual void updateLogic() {}
    virtual void updateTransform() {}

    const std::string& getName() const {
        return name_;
    }

    TransformData* getTransform() {
        return transform_.get();
    }

protected:
    std::unique_ptr<TransformData> transform_;

    void buildTransform() {
        transform_ = std::make_unique<TransformData>(lex_.getC<Lex>("transform", Lex()));
    }

    std::string name_;
    Lex lex_;
};

#endif // I_OBJECT_H
