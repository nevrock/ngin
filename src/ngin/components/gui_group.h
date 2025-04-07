#ifndef GUI_GROUP_H
#define GUI_GROUP_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ngin/log.h>
#include <ngin/ngin.h>


class GuiGroup : public IComponent {
public:
    GuiGroup(const std::string name, const Lex& lex, IObject* parent)
    : IComponent(name, lex, parent) {

    }
    ~GuiGroup() {

    }

    void init() override {

    }
    void launch() override {
        // Implementation of launch method
    }
    void update() override {
        RectData* rect = getRectTransform();
        glm::mat4 rectM = rect->getRectMatrix();
        // Implementation of update method
        std::vector<RectData*> rects = getChildRectTransforms();
        for (auto& rect : rects) {
        }
    }
    void updateLate() override {
        // Implementation of updateLate method
    }
private:
    glm::vec2 spacing_;

};

#endif // GUI_GROUP_H
