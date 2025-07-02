#ifndef ASSET_H
#define ASSET_H

#include <string>
#include <ngin/debug/printer.h>

class Asset {
public:
    Asset(unsigned int id, std::string name) : id_(id), name_(name) {}
    virtual ~Asset() {}
    virtual void read(const std::string& filepath, ngin::debug::Printer& debug) = 0;
    virtual void write(const std::string& filepath) const = 0;
    std::string& get_name() {
        return name_;
    }
protected:
    std::string name_;
    unsigned int id_;
};

#endif // ASSET_H+