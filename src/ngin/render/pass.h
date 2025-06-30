#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include <string>
#include <ngin/render/gl/context.h>

class RenderPass {
public:
    RenderPass(unsigned int id, GlContext& context) : id_(id), context_(context) {
    }
    unsigned int get_id() const { 
        return id_; 
    }
    
    virtual void render() = 0; // Accepts any number of unsigned int arguments
    virtual void setup() = 0;
    virtual void bind() = 0;

protected:
    unsigned int id_; // ID of the render pass
    GlContext& context_;
};

#endif // RENDER_PASS_H
