#ifndef CHAIR_H
#define CHAIR_H

#include "Shader.h"
#include "primitives/Cube.h"
#include <glm/glm.hpp>

class Chair
{
public:
    Chair(const glm::vec3& pos = glm::vec3(0.0f), float rotY = 0.0f, float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;

    // Renders the wooden Adirondack camping lounge chair (Image 2)
    void draw(const Shader& shader, const Cube& unitCube) const;

private:
    glm::vec3 position;
    float rotationY;
    float scaleFactor;

    void drawPart(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                  const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                  float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) const;
};

#endif // CHAIR_H
