#ifndef NGIN_H
#define NGIN_H

#include <ngin/atlas/atlas.h>
#include <ngin/assets/assets.h>

#include <iostream>
#include <string>

#include <ngin/render/data.h>
#include <ngin/render/drawer/data.h>

// Declare functions directly in the Ngin namespace
class NGIN {
public:
    template <typename T>
    static void envset(const std::string& key, T* value) {
        env_.set(key, value);
    }
    template <typename T>
    static T* envget(const std::string& key, T* defaultValue = nullptr) {
        return env_.get<T>(key, defaultValue);
    }
    static inline Assets& assets() {
        return assets_;
    }
    static inline RenderData& render() {
        return render_;
    }
    static inline DrawerData& drawer() {
        return drawer_;
    }
private:
    static inline Atlas env_;

    static inline Assets assets_;
    
    static inline RenderData render_;
    static inline DrawerData drawer_;
};

#endif