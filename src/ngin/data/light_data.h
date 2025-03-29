#ifndef LIGHT_DATA_H
#define LIGHT_DATA_H

#include <fstream> // Add this include directive
#include <sstream> // Add this include directive

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ngin/data/i_data.h>
#include <ngin/lex.h>
#include <ngin/log.h>

class LightData : public IData {
public:
    LightData() {} 
    LightData(const Lex& data) {
        name_ = data.getC<std::string>("name", "");
        position_ = data.getVec("position", glm::vec3(0.0));
        color_ = data.getVec("color", glm::vec3(1.0));
    }
    LightData(const std::string& name, const glm::vec3& position, const glm::vec3& color)
        : name_(name), position_(position)  {}

    void execute() override {}
    std::string getName() override { return name_; } 

    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getColor() const { return color_; }

    void setName(const std::string& name) { name_ = name; }
    void setPosition(const glm::vec3& position) { position_ = position; }
    void setColor(const glm::vec3& color) { color_ = color; }

private:
    std::string name_;

    glm::vec3 position_, color_;
};

#endif