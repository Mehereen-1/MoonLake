#ifndef CAMPFIRE_H
#define CAMPFIRE_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Pyramid.h"
#include "primitives/Sphere.h"
#include "primitives/Rock.h"
#include <glm/glm.hpp>

// Forward declaration of the motion state structure defined in MotionSystem.h
struct FlameMotionState;

class Campfire
{
public:
    Campfire(const glm::vec3& pos = glm::vec3(0.0f), float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getScale() const;

    // Base root world transformation matrix
    glm::mat4 getRootMatrix() const;

    // Dynamic point light source position and flickering golden-orange firelight
    glm::vec3 getLightPosition() const;
    glm::vec3 getLightColor(float time) const;
    float getLightIntensity(float time) const;

    // Primary draw function: Accepts precomputed flame transforms from MotionSystem
    // (Keeps static geometry rendering cleanly decoupled from motion math)
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Rock& unitRock,
              const FlameMotionState& flameMotion) const;

    // Convenience overload for backward compatibility
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Rock& unitRock,
              float time) const;

private:
    glm::vec3 position;
    float scaleFactor;
};

#endif // CAMPFIRE_H
