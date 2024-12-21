#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <unordered_map>
#include <string>
#include <functional>
#include <ngin/gl/window.h>
#include <ngin/node/node.h>

class GlWindow : public Node {
public:
    GlWindow(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
        setupCommandDictionary();
    }

    void update(std::string& pass) override {
        for (const auto& cmd : data_.getC<std::vector<std::string>>("commands", {})) {
            auto command = cmds_.find(cmd);
            if (command != cmds_.end()) {
                command->second();
            }
        }
    }

private:
    std::unordered_map<std::string, std::function<void()>> cmds_;

    void setupCommandDictionary() {
        // Add commands to the dictionary
        cmds_["clear"] = [this]() { Window::getMainWindow()->clear(); };
    }
};

#endif // GL_WINDOW_H
