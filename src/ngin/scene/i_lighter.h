#ifndef I_LIGHTER_H
#define I_LIGHTER_H

#include <ngin/scene/i_component.h>
#include <ngin/data/shader_data.h>

#include <ngin/data/light_data.h>

class ILighter : public IComponent {
public:
    ILighter(const std::string& name, const Lex& lex, IObject* parent) : IComponent(name, lex, parent) {
        lightData_ = std::make_shared<LightData>();
    }
    ~ILighter() {
    }

    std::shared_ptr<LightData> getLightData() { 
        lightData_->setName(getName());
        lightData_->setPosition(getTransform()->getPosition());
        lightData_->setColor(lex_.getVec("color", glm::vec3(1.0)));
        lightData_->setIntensity(lex_.getC<float>("intensity", 1.0f));
        lightData_->setRange(lex_.getC<float>("range", 1.0f));
        return lightData_;
    }

protected:
    std::shared_ptr<LightData> lightData_;
};

#endif // I_LIGHTER_H
