#ifndef TRANSLUCENT_PASS_H
#define TRANSLUCENT_PASS_H

#include <iostream>
#include <functional> // For std::function

#include <ngin/render/render_pass.h>

class TranslucentPass : public RenderPass {
public:
    TranslucentPass(unsigned int id) : RenderPass(id) {}

    void setup() override {

    }
    void render() override {
        // Reset face culling and depth function
        glCullFace(GL_BACK);   
        glDepthFunc(GL_LESS);

        // Render translucent objects
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Drawer::prep("translucent");
        bind();
        Drawer::draw("translucent");
    }
    void bind() override {
        if (shadowBind_) {
            shadowBind_();
        }
    }

    void linkBinding(std::function<void()> shadowBind) {
        shadowBind_ = shadowBind;
    }

private:
    std::function<void()> shadowBind_; // Function to bind shadow

};

#endif // TRANSLUCENT_PASS_H
