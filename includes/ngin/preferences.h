#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <ngin/event.h>
#include <ngin/collections/nevf.h>
#include <memory>
#include <functional>
#include <atomic>

class Preferences {
public:
    static void init() {
        std::cout << "!!!   preferences started   !!!" << std::endl;
    }
    static void setData(std::shared_ptr<Nevf> nevf) {
        data_ = nevf;
        
        glm::vec2 screenSize = data_->getVec2("screen_size", glm::vec2(1024, 576));
        screenWidthTarget = (int)screenSize.x;
        screenHeightTarget = (int)screenSize.y;
    }
    static std::shared_ptr<Nevf> getData() {
        return data_;
    }
    static void addStateListener(std::function<void()> listener) {
        eventState_.addListener(listener);
    }

    static int screenWidthTarget, screenHeightTarget;

private:
    // Constructor, Copy constructor, and Assignment operator are private to prevent multiple instances
    Preferences() {}
    Preferences(const Preferences&) = delete;
    Preferences& operator=(const Preferences&) = delete;

    static Event eventState_;

    static std::shared_ptr<Nevf> data_;
};

#endif // PREFERENCES_H