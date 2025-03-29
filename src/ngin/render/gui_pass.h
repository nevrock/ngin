#ifndef GUI_PASS_H
#define GUI_PASS_H

#include <ngin/render/render_pass.h>
#include <iostream>

class GuiPass : public RenderPass {
public:
    GuiPass(unsigned int id) : RenderPass(id) {}

    void setup() override {

    }
    void render() override {
        glDisable(GL_DEPTH_TEST);
        Context::framebuffer(0);

        Drawer::prep("gui");
        Drawer::draw("gui");

        // Re-enable depth test after rendering GUI
        glEnable(GL_DEPTH_TEST);

        // Disable blending
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);

    }
    void bind() override {

    }

private:

};

#endif // GUI_PASS_H
