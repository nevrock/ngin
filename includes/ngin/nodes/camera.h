#ifndef CAMERA_H
#define CAMERA_H

#include <ngin/node/node.h>
#include <ngin/gl/window.h>
#include <ngin/game.h>

#include <ngin/data/shader_data.h>

class Camera : public Node {
public:
    explicit Camera(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), zoom_(1.0)
    {
    }

    ~Camera() override = default;

    void execute(std::string& pass) override {
        Node::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    
        std::shared_ptr<NodePort> inputPortTransform = getInputPortByType("transform");
        std::shared_ptr<NodePort> inputPortRender = getInputPortByType(pass);

        if (inputPortTransform && inputPortRender) {
            std::shared_ptr<TransformData> parentData = inputPortTransform->getData<TransformData>();
            if (parentData) {
                glm::vec3 worldPos = parentData->getWorldPosition();
                glm::mat4 projection = getProjectionMatrix(Window::width, Window::height, zoom_);
                glm::mat4 view = getViewMatrix(worldPos, parentData->getForward(), parentData->getUp());
                glm::mat4 inverseViewMatrix = glm::inverse(view);
                glm::mat4 inverseProjectionMatrix = glm::inverse(projection);

                std::shared_ptr<Nevf> env = Game::getEnv();
                env->set("camera_project", projection);
                env->set("camera_view", view);
                env->set("camera_inverse_view_matrix", inverseViewMatrix);
                env->set("camera_inverse_projection_matrix", inverseProjectionMatrix);

                std::shared_ptr<ShaderData> shader = inputPortRender->getData<ShaderData>();
                if (shader) {
                    shader->setMat4("M_CAMERA_VIEW", view);
                    shader->setMat4("M_CAMERA_PROJECTION", projection);
                    shader->setMat4("M_CAMERA_I_VIEW", inverseViewMatrix);
                    shader->setMat4("M_CAMERA_I_PROJECTION", inverseProjectionMatrix);
                    shader->setVec3("CAMERA_POS", worldPos);
                    shader->setFloat("CAMERA_NEAR_PLANE", 0.1f);
                    shader->setFloat("CAMERA_FAR_PLANE", 100.0f);
                    
                    //std::cout << "camera execute has input port render gui, set shader vars" << std::endl;
                } 
            } 
        } 
        
    }
    void setup() override {
        Node::setup();
    }

private:    
    float zoom_;

    glm::mat4 getViewMatrix(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
    {
        return glm::lookAt(position, position + forward, up);
    }

    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight, float zoom)
    {
        glm::mat4 projection = glm::perspective(glm::radians(zoom), screenWidth / screenHeight, 0.1f, 100.0f);
        return projection;
    }
};

#endif // OBJECT_GRAPH_H