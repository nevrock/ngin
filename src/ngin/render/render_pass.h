#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include <string>
#include <initializer_list>

#include <ngin/render/context.h>
#include <ngin/drawer.h>

class RenderPass {
public:
    RenderPass(unsigned int id) : id_(id) {
        
    }

    unsigned int getId() const { 
        return id_; 
    }
    
    virtual void render() = 0; // Accepts any number of unsigned int arguments
    virtual void setup() = 0;
    virtual void bind() = 0;

private:
    unsigned int id_; // ID of the render pass
};

#endif // RENDER_PASS_H
