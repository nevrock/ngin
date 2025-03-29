#ifndef LIGHT_POINT_H
#define LIGHT_POINT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <glm/glm.hpp>
#include <ngin/constants.h>

#include <ngin/scene/i_lighter.h>    
#include <ngin/lighter.h>    

class LightPoint : public ILighter {
public:
    LightPoint(const std::string name, const Lex& lex, IObject* parent)
        : ILighter(name, lex, parent) {
        Log::console("LightPoint created", 1);
        Lighter::registerLighter(*this);
    }
    ~LightPoint() {
        Lighter::unregisterLighter(*this);
    }

    void init() override {
        // Implementation of init method
    }

    void launch() override {
        // Implementation of launch method
    }

    void update() override {
        // Implementation of update method
    }

    void updateLate() override {
        // Implementation of updateLate method
    }

private:
    // ...existing code...
};

#endif // LIGHT_POINT_H
