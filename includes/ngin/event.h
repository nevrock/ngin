#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <vector>
#include <functional>

class Event {
public:
    void addListener(std::function<void()> listener) {
        listeners.push_back(listener);
    }
    void trigger() {
        for (auto& listener : listeners) {
            listener();
        }
    }

private:
    std::vector<std::function<void()>> listeners;
};

#endif // EVENT_H