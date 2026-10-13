#include "Camera.h"
#include <cmath>

Camera::Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPos),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      worldUp(startUp),
      yaw(startYaw),
      pitch(startPitch),
      movementSpeed(SPEED),
      mouseSensitivity(SENSITIVITY),
      zoom(ZOOM),
      clampYaw(false)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::setLookAt(const glm::vec3& eye, const glm::vec3& target)
{
    position = eye;
    glm::vec3 dir = glm::normalize(target - eye);
    pitch = glm::degrees(std::asin(glm::clamp(dir.y, -0.999f, 0.999f)));
    yaw   = glm::degrees(std::atan2(dir.z, dir.x));
    updateCameraVectors();
}

void Camera::setOrientation(float newYaw, float newPitch)
{
    yaw = newYaw;
    pitch = newPitch;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime, bool fast)
{
    float currentSpeed = movementSpeed * (fast ? 2.5f : 1.0f);
    float velocity = currentSpeed * deltaTime;

    if (direction == CAM_FORWARD)
        position += front * velocity;
    if (direction == CAM_BACKWARD)
        position -= front * velocity;
    if (direction == CAM_LEFT)
        position -= right * velocity;
    if (direction == CAM_RIGHT)
        position += right * velocity;
    if (direction == CAM_UP)
        position += worldUp * velocity;
    if (direction == CAM_DOWN)
        position -= worldUp * velocity;
}

void Camera::processKeyboardRotation(float yawDelta, float pitchDelta, float deltaTime)
{
    const float rotSpeed = 70.0f; // degrees per second
    yaw   += yawDelta * rotSpeed * deltaTime;
    pitch += pitchDelta * rotSpeed * deltaTime;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // while (yaw > 180.0f)  yaw -= 360.0f;
    // while (yaw < -180.0f) yaw += 360.0f;

    if (clampYaw)
    {
        const float minYaw = -150.0f; // Left limit (cabin)
        const float maxYaw =  -20.0f; // Right limit (cave)
        if (yaw < minYaw) yaw = minYaw;
        if (yaw > maxYaw) yaw = maxYaw;
    }

    updateCameraVectors();
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    if (clampYaw)
    {
        const float minYaw = -150.0f; // Left limit (cabin)
        const float maxYaw =  -20.0f; // Right limit (cave)
        if (yaw < minYaw) yaw = minYaw;
        if (yaw > maxYaw) yaw = maxYaw;
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
    // Scroll wheel physically moves camera closer or farther (Dolly)
    processDolly(yoffset * 1.5f);
}

void Camera::processDolly(float distance)
{
    position += front * distance;
}

void Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    newFront.y = std::sin(glm::radians(pitch));
    newFront.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
