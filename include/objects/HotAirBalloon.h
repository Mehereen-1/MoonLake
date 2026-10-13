#ifndef HOT_AIR_BALLOON_H
#define HOT_AIR_BALLOON_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"
#include "primitives/Pyramid.h"
#include <glm/glm.hpp>

class HotAirBalloon
{
public:
    HotAirBalloon(const glm::vec3& startPos = glm::vec3(12.0f, 4.8f, 2.0f),
                  float startRotY = -45.0f,
                  float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;
    float getForwardSpeed() const;
    float getVerticalSpeed() const;
    bool isBurnerActive() const;

    // Movement & Kinematic Flight Controls
    void processInput(bool forward, bool backward,
                      bool turnLeft, bool turnRight,
                      bool ascend, bool descend,
                      bool boost, float dt);

    void update(float dt, float totalTime);
    void constrainToBounds();

    // Hierarchical Root Transformation Matrix
    glm::mat4 getRootMatrix() const;

    // Third-Person Follow Camera Views
    glm::vec3 getCameraFollowPosition() const;
    glm::vec3 getCameraLookTarget() const;

    // First-Person Gondola/Basket Passenger View
    glm::vec3 getBasketCameraPosition() const;
    glm::vec3 getBasketCameraLookTarget() const;

    // Main Draw Function using standard unit primitives
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              const Pyramid& unitPyramid) const;

private:
    glm::vec3 m_position;       // Position of basket base in world space
    float m_rotationY;          // Heading angle in degrees
    float m_scaleFactor;

    // Flight velocities
    float m_forwardSpeed;       // Cruise velocity (m/s)
    float m_verticalSpeed;      // Ascent/Descent velocity (m/s)
    float m_maxForwardSpeed;
    float m_maxVerticalSpeed;
    float m_turnSpeed;

    // Aerodynamic swaying & burner animation
    bool m_isBurning;           // True when burner is firing (SPACE pressed or boost)
    float m_flameFlicker;       // Harmonic flame scale modulation
    float m_currentBobY;        // Gentle atmospheric wave bobbing
    float m_swayPitch;          // Longitudinal tilt due to forward momentum
    float m_swayRoll;           // Transverse atmospheric sway
    float m_totalTime;

    // Helper draw routines for unit primitives
    void drawPart(const Shader& shader,
                  const Cube& unitCube,
                  const glm::mat4& parentMat,
                  const glm::vec3& localOffset,
                  const glm::vec3& localScale,
                  const glm::vec3& color,
                  float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                  bool emissive = false,
                  const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    void drawCylinderPart(const Shader& shader,
                          const Cylinder& unitCylinder,
                          const glm::mat4& parentMat,
                          const glm::vec3& localOffset,
                          const glm::vec3& localScale,
                          const glm::vec3& color,
                          float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) const;

    void drawSpherePart(const Shader& shader,
                        const Sphere& unitSphere,
                        const glm::mat4& parentMat,
                        const glm::vec3& localOffset,
                        const glm::vec3& localScale,
                        const glm::vec3& color,
                        bool emissive = false,
                        const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    void drawPyramidPart(const Shader& shader,
                         const Pyramid& unitPyramid,
                         const glm::mat4& parentMat,
                         const glm::vec3& localOffset,
                         const glm::vec3& localScale,
                         const glm::vec3& color,
                         float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                         bool emissive = false,
                         const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    // Hierarchical Component Draw Passes
    void drawBasket(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder, const Sphere& unitSphere, const glm::mat4& rootMat) const;
    void drawBurner(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder, const Pyramid& unitPyramid, const Sphere& unitSphere, const glm::mat4& rootMat) const;
    void drawRigging(const Shader& shader, const Cylinder& unitCylinder, const glm::mat4& rootMat) const;
    void drawEnvelope(const Shader& shader, const Cylinder& unitCylinder, const Sphere& unitSphere, const glm::mat4& rootMat) const;
};

#endif // HOT_AIR_BALLOON_H
