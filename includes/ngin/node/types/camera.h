#ifndef CAMERA_H
#define CAMERA_H

#include <ngin/node/node.h>
#include <ngin/gl/window.h>
#include <ngin/game.h>

class Camera : public Node {
public:
    explicit Camera(const std::string& name, Nevf& dictionary)
        : Node(name, dictionary), zoom_(1.0)
    {
    }

    ~Camera() override = default;

    void execute(std::string& pass) override {
        Node::execute(pass); // Correctly calls the base class execute(), which retrieves data so we are ready to extract
    
        if (pass == "transform") {
            std::shared_ptr<NodePort> inputPort = getInputPortByType("transform");
            if (inputPort) {
                std::shared_ptr<TransformData> parentData = inputPort->getData<TransformData>();
                if (parentData) {
                    glm::mat4 projection = getProjectionMatrix(Window::width, Window::height, zoom_);
                    glm::mat4 view = getViewMatrix(parentData->getWorldPosition(), parentData->getForward(), parentData->getUp());
                    glm::mat4 inverseViewMatrix = glm::inverse(view);
                    glm::mat4 inverseProjectionMatrix = glm::inverse(projection);

                    std::shared_ptr<Nevf> env = Game::getEnv();
                    env->set("camera_project", projection);
                    env->set("camera_view", view);
                    env->set("camera_inverse_view_matrix", inverseViewMatrix);
                    env->set("camera_inverse_projection_matrix", inverseProjectionMatrix);
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