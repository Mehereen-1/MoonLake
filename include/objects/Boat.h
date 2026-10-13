#ifndef BOAT_H
#define BOAT_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/TriangularPrism.h"
#include "primitives/Cylinder.h"
#include <glm/glm.hpp>

class Boat
{
public:
    Boat(const glm::vec3& startPos = glm::vec3(3.8f, -1.14f, 8.0f),
         float startRotY = -15.0f,
         float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;
    float getSpeed() const;

    // Movement Physics & Control (Called each frame with dt)
    void processInput(bool forward, bool backward, bool left, bool right, float dt);
    void update(float dt, float totalTime);

    // Lake Boundary Constraint (Keeps boat safely in water basin)
    void constrainToLake();

    // Hierarchical Root Transformation Matrix: M = T(pos + bobbing) * R(rotY + rocking) * S(scale)
    glm::mat4 getRootMatrix() const;

    // Third-Person Follow Camera Target & Position:
    // CameraPosition = BoatPosition + R(theta) * CameraOffset
    glm::vec3 getCameraFollowPosition() const;
    glm::vec3 getCameraLookTarget() const;

    // Primary draw function using project unit primitives
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Cylinder& unitCylinder) const;

    // Backward-compatible overload
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism) const;

private:
    glm::vec3 position;       // Center reference point P0 on the water
    float rotationY;          // Heading angle theta (degrees)
    float scaleFactor;

    // Movement state
    float speed;              // Current forward/backward velocity (m/s)
    float maxSpeed;           // Cruise velocity (m/s)
    float turnSpeed;          // Yaw rate (deg/s)

    // Floating bobbing & wave rocking state
    float currentBobY;        // Vertical wave bobbing offset A * sin(omega * t)
    float currentPitch;       // Longitudinal rocking
    float currentRoll;        // Transverse rocking
    float oarSwingAngle;      // Periodic oar sweep angle
    float totalTimeElapsed;

    // Transformation helper for component pieces
    void drawPart(const Shader& shader,
                  const Cube& unitCube,
                  const glm::mat4& parentMat,
                  const glm::vec3& localOffset,
                  const glm::vec3& localScale,
                  const glm::vec3& color,
                  float rotY = 0.0f,
                  float rotX = 0.0f,
                  float rotZ = 0.0f,
                  bool emissive = false,
                  const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    void drawTriangularPart(const Shader& shader,
                            const TriangularPrism& unitPrism,
                            const glm::mat4& parentMat,
                            const glm::vec3& localOffset,
                            const glm::vec3& localScale,
                            const glm::vec3& color,
                            float rotY = 0.0f,
                            float rotX = 0.0f,
                            float rotZ = 0.0f) const;

    void drawCylinderPart(const Shader& shader,
                          const Cylinder& unitCylinder,
                          const glm::mat4& parentMat,
                          const glm::vec3& localOffset,
                          const glm::vec3& localScale,
                          const glm::vec3& color,
                          float rotY = 0.0f,
                          float rotX = 0.0f,
                          float rotZ = 0.0f) const;

    // Hierarchical Boat Component Draw Passes
    void drawHull(const Shader& shader, const Cube& unitCube, const TriangularPrism& unitPrism, const glm::mat4& rootMat) const;
    void drawSeats(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const;
    void drawOars(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder, const glm::mat4& rootMat) const;
    void drawDetails(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const;
};

#endif // BOAT_H
