#include "camera.h"
#include <ngin/constants.h>
Camera* Camera::mainCamera = nullptr;


Camera::Camera(Object* parent) 
    : Component(parent) {
    
    Yaw = YAW;
    Pitch = PITCH;
    MovementSpeed = SPEED;
    MouseSensitivity = SENSITIVITY;
    Zoom = ZOOM;

    mainWindow = Window::getMainWindow();
}

void Camera::loadFromDict(const Dict& d) {
    if (d.contains("is_main")) {
        if (d.getC<bool>("is_main", false)) {
            setMainCamera(this);
        }
    }
    if (d.contains("yaw")) {
        Yaw = d.getC<float>("yaw", 0.0);
    }
    if (d.contains("pitch")) {
        Pitch = d.getC<float>("pitch", 0.0);
    }

    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    updateCameraVectors();
}

void Camera::update() {
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix()
{
    glm::vec3 position = getPoint().getPosition();
    return glm::lookAt(position, position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float screenWidth, float screenHeight)
{
    glm::mat4 projection = glm::perspective(glm::radians(Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    return projection;
}

void Camera::updatePreRender(const unsigned int index, Shader& shader) {
    if (index > 1) {
        glm::mat4 projection = getProjectionMatrix(snorri::SCREEN_WIDTH, snorri::SCREEN_HEIGHT);
        glm::mat4 view = getViewMatrix();
        glm::mat4 inverseViewMatrix = glm::inverse(view);
        glm::mat4 inverseProjectionMatrix = glm::inverse(projection);

        shader.setMat4("M_CAMERA_VIEW", view);
        shader.setMat4("M_CAMERA_PROJECTION", projection);
        shader.setMat4("M_CAMERA_I_VIEW", inverseViewMatrix);
        shader.setMat4("M_CAMERA_I_PROJECTION", inverseProjectionMatrix);
        shader.setMat4("M_CAMERA_I_PROJECTION", inverseProjectionMatrix);
        shader.setVec3("CAMERA_POS", getPoint().getPosition());
        shader.setFloat("CAMERA_NEAR_PLANE", 0.1f);
        shader.setFloat("CAMERA_FAR_PLANE", 100.0f);
    }
}
Camera* Camera::getMainCamera() {
    return mainCamera;
}
void Camera::setMainCamera(Camera* Camera) {
    mainCamera = Camera;
}


// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    Point& p = getPoint();
    glm::vec3 position = p.getPosition();
    if (direction == FORWARD)
        p.setPosition(position + Front * velocity);
    if (direction == BACKWARD)
        p.setPosition(position - Front * velocity);
    if (direction == LEFT)
        p.setPosition(position - Right * velocity);
    if (direction == RIGHT)
        p.setPosition(position + Right * velocity);

}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::processMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up    = glm::normalize(glm::cross(Right, Front));
}

bool camera_registered = []() {
    Log::console("camera registering now");
    Object::registerComponent("camera", [](Object* parent) {
        return std::make_shared<Camera>(parent);
    });
    return true;
}();
