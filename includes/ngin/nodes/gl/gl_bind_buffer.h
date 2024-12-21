#ifndef GL_BIND_BUFFER_H
#define GL_BIND_BUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ngin/node/node.h>
#include <ngin/game.h>

class GlBindBuffer : public Node {
public:
    explicit GlBindBuffer(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary) {
        if (dictionary.contains("buffer_index")) {
            bufferIndex_ = dictionary.getC<int>("buffer_index", 0);
        } else if (dictionary.contains("buffer_name")) {
            std::string bufferName = dictionary.getC<std::string>("buffer_name", "");
            bufferIndex_ = Game::env<unsigned int>(bufferName);
        } else {
            throw std::runtime_error("No buffer_index or buffer_name found in dictionary");
        }
    }

    ~GlBindBuffer() override = default;

    void update(std::string& pass) override {
        Node::update(pass);
        glBindFramebuffer(GL_FRAMEBUFFER, bufferIndex_);
    }

    void start(std::string& pass) override {
        Node::start(pass);
    }

private:
    unsigned int bufferIndex_;
};

#endif // GL_BIND_BUFFER_H
