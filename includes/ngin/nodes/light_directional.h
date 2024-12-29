#ifndef LIGHT_DIRECTIONAL_H
#define LIGHT_DIRECTIONAL_H

#include <ngin/node/node.h>
#include <ngin/gl/window.h>
#include <ngin/game.h>

#include <ngin/data/render_data.h>
#include <ngin/data/transform_data.h>

class LightDirectional : public Node {
public:
    explicit LightDirectional(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary)
    {
    }

    ~LightDirectional() override = default;

    void execute(std::string& pass) override {
        retrieveInputData(pass);
        update(pass);
    }

    void update(std::string& pass) override {
        Node::update(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    
        if (pass.find("render") == std::string::npos) {
            // Only execute on render passes
            setOutputData(pass);
            return;
        }

        std::shared_ptr<NodePort> inputPortTransform = getInputPortByType("transform");
        std::shared_ptr<NodePort> inputPortRender = getInputPortByType(pass);

        if (inputPortTransform && inputPortRender) {
            std::shared_ptr<TransformData> parentData = inputPortTransform->getData<TransformData>();
            if (parentData) {
                std::shared_ptr<RenderData> shader = inputPortRender->getData<RenderData>();
                if (shader) {
                    ShaderData& shaderData = shader->getShader();

                    glm::mat4 lightProjection, lightView;
                    glm::mat4 lightSpaceMatrix;

                    glm::vec3 lightDir = parentData->getFrontDirection();  // Use a normalized direction vector for the light

                    float nearPlane = 1.0f, farPlane = 45.0f;  // Adjust these to ensure your scene fits within the light's frustum
                    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);

                    glm::vec3 arbitraryCenter = glm::vec3(5.0f);  // You might need to adjust this to be the center of your scene
                    lightView = glm::lookAt(arbitraryCenter - lightDir * 10.0f, arbitraryCenter, glm::vec3(0.0, 1.0, 0.0));
                    lightSpaceMatrix = lightProjection * lightView;

                    shaderData.setMat4("M_LIGHT_SPACE", lightSpaceMatrix);
                    shaderData.setVec3("LIGHT_DIRECTION", lightDir);
                    shaderData.setVec3("LIGHT_POS", parentData->getPosition());
                    shaderData.setFloat("LIGHT_FAR_PLANE", farPlane);
                    shaderData.setFloat("LIGHT_NEAR_PLANE", nearPlane);
                    
                    setOutputData(pass, shader);
                } 
            } else {
            }
        } 
        
    }
    void setup() override {
        Node::setup();
    }

private:    
};

#endif // LIGHT_DIRECTIONAL_H