#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include <string>

class RenderPass {
public:
    RenderPass(unsigned int id) : id_(id) {
        
    }

    unsigned int getId() const { 
        return id_; 
    }
    
    virtual void render() = 0;
    virtual void setup() = 0;
    virtual void bind() = 0;

private:
    unsigned int id_; // ID of the render pass
};

#endif // RENDER_PASS_H
