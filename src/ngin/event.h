#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <vector>
#include <functional>

#include <ngin/lex.h>

class Event {
public:
    void addListener(std::function<void(const Lex&)> listener) {
        listeners.push_back(listener);
    }
    void trigger(const Lex& args) {
        for (auto& listener : listeners) {
            listener(args);
        }
    }
    void trigger() {
        Lex emptyArgs;
        trigger(emptyArgs);
    }

private:
    std::vector<std::function<void(const Lex&)>> listeners;
};

#endif // EVENT_H