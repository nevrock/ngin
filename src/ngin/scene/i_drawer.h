#ifndef I_DRAWER_H
#define I_DRAWER_H

#include <ngin/scene/i_component.h>
#include <ngin/data/shader_data.h>

class IDrawer : public IComponent {
public:
    IDrawer(const std::string& name, const Lex& lex, IObject* parent) : IComponent(name, lex, parent) {
    }
    ~IDrawer() {
    }
    
    virtual void prep(ShaderData& shader) = 0; // Pure virtual method for preparing
    virtual void draw(ShaderData& shader) = 0; // Pure virtual method for rendering
};

#endif // I_DRAWER_H
