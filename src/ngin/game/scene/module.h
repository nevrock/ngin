#ifndef MODULE_H
#define MODULE_H

#include <memory>
#include <string>

#include <ngin/atlas/atlas.h>
#include <ngin/debug/logger.h>

class Object;

class Module {
public:
    Module() = default;
    Module(const std::string name, Object& parent, Atlas& data) 
        : name_(name), owner_(parent) {
            logger_ = new Logger("Module::" + name);
    }
    

    std::string get_name() const { return name_; }

    virtual void init() = 0;
    virtual void launch() = 0;
    virtual void update() = 0;
    virtual void update_late() = 0;

protected:
    Logger* logger_;
    Object& owner_;
    std::string name_;
};

#endif // MODULE_H
