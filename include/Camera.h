#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
    CAM_FORWARD,
    CAM_BACKWARD,
    CAM_LEFT,
    CAM_RIGHT,
    CAM_UP,
    CAM_DOWN
};

const float YAW         = -90.0f;
const float PITCH       = -6.0f;
const float SPEED       =  5.5f;
const float SENSITIVITY =  0.10f;
const float ZOOM        =  45.0f;

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
    bool clampYaw;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 2.2f, 9.5f),
           glm::vec3 startUp  = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw     = YAW,
           float startPitch   = PITCH);

    glm::mat4 getViewMatrix() const;

    void setLookAt(const glm::vec3& eye, const glm::vec3& target);
    void setOrientation(float newYaw, float newPitch);
    void processKeyboard(CameraMovement direction, float deltaTime, bool fast = false);
    void processKeyboardRotation(float yawDelta, float pitchDelta, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    void processDolly(float distance);
    void updateCameraVectors();
};

#endif // CAMERA_H
