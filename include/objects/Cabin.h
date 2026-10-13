#ifndef CABIN_H
#define CABIN_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/TriangularPrism.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"
#include "primitives/Pyramid.h"
#include <glm/glm.hpp>
#include <vector>

class Cabin
{
public:
    // Creates an expanded rustic camping cabin with openable door, vintage interior,
    // and stone fireplace with smoke-emitting chimney.
    Cabin(const glm::vec3& pos = glm::vec3(-12.5f, -0.72f, -2.0f),
          float rotY = 22.0f,
          float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;

    // Interactive Door Control & State
    void toggleDoor();
    void setDoorOpen(bool open);
    bool isDoorOpen() const;
    float getDoorAngle() const;

    // Fireplace Control & State
    void setFireLit(bool lit);
    void toggleFire();
    bool isFireLit() const;
    void addWood();
    int getWoodCount() const;

    // World Interaction Query Helpers (Door, Fireplace, Chimney)
    glm::vec3 getDoorWorldPosition() const;
    glm::vec3 getFireplaceWorldPosition() const;
    glm::vec3 getChimneyWorldPosition() const;

    // Dynamic Fireplace Point Light
    glm::vec3 getFireplaceLightPosition() const;
    glm::vec3 getFireplaceLightColor(float time) const;
    float getFireplaceLightIntensity(float time) const;

    // Animation & Physics Update (Door swinging, flame flicker, chimney smoke)
    void update(float dt, float totalTime);

    // Root World Transformation Matrix
    glm::mat4 getRootMatrix() const;

    // Primary Hierarchical Draw Function using Unit Primitives
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              const Pyramid& unitPyramid) const;

    // Backward-compatible overloads
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Cylinder& unitCylinder) const;

    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism) const;

    void draw(const Shader& shader,
              const Cube& unitCube) const;

private:
    glm::vec3 m_position;
    float m_rotationY;
    float m_scaleFactor;

    // Door animation state
    bool m_isDoorOpen;
    float m_doorAngle;          // Current rotation angle in degrees (0 = closed, -95 = open)
    float m_targetDoorAngle;

    // Fireplace & wood state
    bool m_isFireLit;
    int m_woodPlacedCount;
    float m_totalTime;
    float m_flameFlicker;

    // Component Draw Helpers
    void drawCubePart(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                      const glm::vec3& localOffset, const glm::vec3& localScale,
                      const glm::vec3& color,
                      float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                      bool isEmissive = false, const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    void drawTriangularPrismPart(const Shader& shader, const TriangularPrism& unitPrism, const glm::mat4& parentMat,
                                const glm::vec3& localOffset, const glm::vec3& localScale,
                                const glm::vec3& color,
                                float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) const;

    void drawCylinderPart(const Shader& shader, const Cylinder& unitCylinder, const glm::mat4& parentMat,
                          const glm::vec3& localOffset, const glm::vec3& localScale,
                          const glm::vec3& color,
                          float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) const;

    void drawSpherePart(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                        const glm::vec3& localOffset, const glm::vec3& localScale,
                        const glm::vec3& color,
                        bool isEmissive = false, const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    void drawPyramidPart(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
                         const glm::vec3& localOffset, const glm::vec3& localScale,
                         const glm::vec3& color,
                         float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                         bool isEmissive = false, const glm::vec3& emitColor = glm::vec3(0.0f)) const;

    // Sub-Assembly Renderers
    void drawExterior(const Shader& shader, const Cube& unitCube, const TriangularPrism& unitPrism,
                      const Cylinder& unitCylinder, const glm::mat4& rootMat) const;

    void drawInteractiveDoor(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const;

    void drawFireplace(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                       const Pyramid& unitPyramid, const Sphere& unitSphere, const glm::mat4& rootMat) const;

    void drawChimneySmoke(const Shader& shader, const Sphere& unitSphere, const glm::mat4& rootMat) const;

    void drawInteriorFurniture(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                               const Sphere& unitSphere, const glm::mat4& rootMat) const;

    void drawWindowsGlass(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const;
};


#endif // CABIN_H
