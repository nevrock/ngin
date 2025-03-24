// model_drawer.h

#ifndef LIGHTER_H
#define LIGHTER_H

#include <vector>
#include <string>
#include <functional> // Include for std::reference_wrapper
#include <glm/glm.hpp>

#include <ngin/resources.h> // Include ILighter
#include <ngin/scene/i_lighter.h> // Include ILighter
#include <ngin/data/light_data.h> // Include ILighter

class Lighter {
public:
    static void init() {
    }

    static void start() {
    }

    static void registerLighter(ILighter& drawer) {
        Log::console("Registering lighter: " + drawer.getName(), 1);
        lighters_.push_back(drawer);
    }

    static void unregisterLighter(ILighter& drawer) {
        lighters_.erase(std::remove_if(lighters_.begin(), lighters_.end(), [&drawer](std::reference_wrapper<ILighter> d) {
            return &d.get() == &drawer;
        }), lighters_.end());
    }

    static void update(const std::string& shaderName) {
        ShaderData& shader = Resources::getShaderData(shaderName);
        shader.use();
        std::vector<std::shared_ptr<LightData>> lights;
        //Log::console("updating lights, count: " + std::to_string(lighters_.size()));
        for (auto& lighter : lighters_) {
            std::shared_ptr<LightData> data = lighter.get().getLightData();
            lights.push_back(data);
        }
        int lightCount = lights.size();
        for (unsigned int i = 0; i < lightCount; i++) {
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", lights[i]->getPosition());
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lights[i]->getColor());
        
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lights[i]->getPosition());
            model = glm::scale(model, glm::vec3(0.0f));
            shader.setMat4("model", model);
            shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            
            renderSphere();
        }
    }
    static void updateDeferred(const std::string& shaderName) {
        ShaderData& shader = Resources::getShaderData(shaderName);
        shader.use();
        std::vector<std::shared_ptr<LightData>> lights;
        //Log::console("updating lights, count: " + std::to_string(lighters_.size()));
        for (auto& lighter : lighters_) {
            std::shared_ptr<LightData> data = lighter.get().getLightData();
            lights.push_back(data);
        }
        for (unsigned int i = 0; i < lights.size(); i++)
        {
            std::shared_ptr<LightData> data = lights[i];
            shader.setVec3("lights[" + std::to_string(i) + "].Position", data->getPosition());
            shader.setVec3("lights[" + std::to_string(i) + "].Color", data->getColor());
            // update attenuation parameters and calculate radius
            const float linear = 0.7f;
            const float quadratic = 1.8f;
            shader.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
            shader.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
        }
    }
    

    // renders (and builds at first invocation) a sphere
    // -------------------------------------------------
    static inline unsigned int sphereVAO_;
    static inline unsigned int indexCount_;
    static void renderSphere()
    {
        if (sphereVAO_ == 0)
        {
            glGenVertexArrays(1, &sphereVAO_);

            unsigned int vbo, ebo;
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);

            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> uv;
            std::vector<glm::vec3> normals;
            std::vector<unsigned int> indices;

            const unsigned int X_SEGMENTS = 64;
            const unsigned int Y_SEGMENTS = 64;
            const float PI = 3.14159265359f;
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
                {
                    float xSegment = (float)x / (float)X_SEGMENTS;
                    float ySegment = (float)y / (float)Y_SEGMENTS;
                    float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                    float yPos = std::cos(ySegment * PI);
                    float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                    positions.push_back(glm::vec3(xPos, yPos, zPos));
                    uv.push_back(glm::vec2(xSegment, ySegment));
                    normals.push_back(glm::vec3(xPos, yPos, zPos));
                }
            }

            bool oddRow = false;
            for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
            {
                if (!oddRow) // even rows: y == 0, y == 2; and so on
                {
                    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                    {
                        indices.push_back(y * (X_SEGMENTS + 1) + x);
                        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    }
                }
                else
                {
                    for (int x = X_SEGMENTS; x >= 0; --x)
                    {
                        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                        indices.push_back(y * (X_SEGMENTS + 1) + x);
                    }
                }
                oddRow = !oddRow;
            }
            indexCount_ = static_cast<unsigned int>(indices.size());

            std::vector<float> data;
            for (unsigned int i = 0; i < positions.size(); ++i)
            {
                data.push_back(positions[i].x);
                data.push_back(positions[i].y);
                data.push_back(positions[i].z);
                if (normals.size() > 0)
                {
                    data.push_back(normals[i].x);
                    data.push_back(normals[i].y);
                    data.push_back(normals[i].z);
                }
                if (uv.size() > 0)
                {
                    data.push_back(uv[i].x);
                    data.push_back(uv[i].y);
                }
            }
            glBindVertexArray(sphereVAO_);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
            unsigned int stride = (3 + 2 + 3) * sizeof(float);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        }

        glBindVertexArray(sphereVAO_);
        glDrawElements(GL_TRIANGLE_STRIP, indexCount_, GL_UNSIGNED_INT, 0);
    }

private:
    inline static std::vector<std::reference_wrapper<ILighter>> lighters_;
};

#endif
