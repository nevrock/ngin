#ifndef MODEL_DRAWER_H
#define MODEL_DRAWER_H

#include <ngin/resources.h>
#include <ngin/log.h>
#include <ngin/nodes/i_drawer.h> // Include IDrawer
#include <ngin/drawer.h> // Include IDrawer

#include <memory>
#include <vector>

class ModelDrawer : public IDrawer, public std::enable_shared_from_this<ModelDrawer> { // Inherit from IDrawer and enable_shared_from_this
public:
    explicit ModelDrawer(const std::string& name, Nevf& dictionary)
        : IDrawer(name, dictionary) // Initialize IDrawer
    {
    }

    ~ModelDrawer() override {
        if (isAddedToDrawer_)
            Drawer::unregisterDrawer(shaderName_, ModelDrawer::shared_from_this());
    }

    void setup() override { 
        IDrawer::setup();
    }

    void launch() override { 
        IDrawer::launch();
        if (isCull_)
            return;

        Drawer::registerDrawer(shaderName_, ModelDrawer::shared_from_this());
    }

    void execute(std::string& pass) override {
        IDrawer::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    }

    void render(ShaderData& shader) override {
        // Implement the render method
    }

protected:
    bool isAddedToDrawer_ = false;
    bool isCull_ = false;

    std::string shaderName_;
};

#endif  // MODEL_DRAWER_H
