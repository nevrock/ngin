#ifndef ASSET_H
#define ASSET_H

#include <string>

class Asset {
public:
    Asset(std::string name) : name_(name) {}
    virtual ~Asset() {}
    virtual void read(const std::string& filepath) = 0;
    virtual void write(const std::string& filepath) const = 0;
    std::string& get_name() {
        return name_;
    }
protected:
    std::string name_;
};

#endif // ASSET_H+