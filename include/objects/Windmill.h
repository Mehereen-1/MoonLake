#ifndef WINDMILL_H
#define WINDMILL_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Pyramid.h"
#include "primitives/Sphere.h"
#include <glm/glm.hpp>

// Forward declaration of the motion state defined in MotionSystem.h
struct WindmillMotionState;

class Windmill
{
public:
    Windmill(const glm::vec3& pos = glm::vec3(0.0f), float rotY = 0.0f, float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;

    // Root world transformation matrix
    glm::mat4 getRootMatrix() const;

    // Primary draw pass receiving precalculated rotor rotation from MotionSystem
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const WindmillMotionState& motionState) const;

    // Backward compatibility overload
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere) const;

private:
    glm::vec3 position;
    float rotationY;
    float scaleFactor;

    // Drawing helper methods using unit primitives with local transformations
    void drawCube(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                  const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                  float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                  bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawPyramid(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
                     const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                     float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                     bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                    const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                    bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    // Sub-assemblies
    void drawBaseAndTower(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                          const glm::mat4& rootMat) const;

    void drawBalconyAndCap(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                           const Sphere& unitSphere, const glm::mat4& rootMat) const;

    void drawRotorAndSails(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                           const Sphere& unitSphere, const glm::mat4& rootMat, float rotationAngle) const;
};

#endif // WINDMILL_H
