#ifndef CLOUD_H
#define CLOUD_H

#include "Shader.h"
#include "primitives/Sphere.h"
#include <glm/glm.hpp>
#include <vector>

struct CloudPuff
{
    glm::vec3 localOffset;
    glm::vec3 localScale;
};

class Cloud
{
public:
    // Constructor allows different cloud archetypes (type 0, 1, 2)
    Cloud(const glm::vec3& startPos, const glm::vec3& cloudScale, float speed = 0.35f, int type = 0);

    // Update horizontal drift with screen wrapping
    void update(float dt);

    // Render using hierarchical modeling with a shared Sphere mesh
    void draw(const Shader& shader, const Sphere& sharedSphere) const;

    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const;

private:
    glm::vec3 position;
    glm::vec3 scale;
    float driftSpeed;
    float minX;
    float maxX;

    std::vector<CloudPuff> puffs;
};

#endif // CLOUD_H
