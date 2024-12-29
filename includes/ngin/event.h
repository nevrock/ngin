#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <vector>
#include <functional>
#include <ngin/collections/nevf.h>

class Event {
public:
    void addListener(std::function<void(const Nevf&)> listener) {
        listeners.push_back(listener);
    }
    void trigger(const Nevf& args) {
        for (auto& listener : listeners) {
            listener(args);
        }
    }
    void trigger() {
        Nevf emptyArgs;
        trigger(emptyArgs);
    }

private:
    std::vector<std::function<void(const Nevf&)>> listeners;
};

#endif // EVENT_H