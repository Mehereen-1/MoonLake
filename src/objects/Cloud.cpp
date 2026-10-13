#include "objects/Cloud.h"
#include <glm/gtc/matrix_transform.hpp>

Cloud::Cloud(const glm::vec3& startPos, const glm::vec3& cloudScale, float speed, int type)
    : position(startPos),
      scale(cloudScale),
      driftSpeed(speed),
      minX(-65.0f),
      maxX(65.0f)
{
    if (type == 0)
    {
        // Type 0: Classic Wide Cumulus (6 overlapping puffs)
        puffs = {
            { glm::vec3( 0.0f,  0.20f,  0.0f), glm::vec3(1.40f, 0.90f, 1.10f) }, // Center mass
            { glm::vec3(-1.1f,  0.00f,  0.1f), glm::vec3(1.10f, 0.75f, 0.90f) }, // Left body
            { glm::vec3( 1.0f,  0.10f, -0.1f), glm::vec3(1.15f, 0.80f, 0.95f) }, // Right body
            { glm::vec3(-1.9f, -0.15f,  0.0f), glm::vec3(0.80f, 0.55f, 0.70f) }, // Left tail
            { glm::vec3( 1.9f, -0.10f,  0.05f), glm::vec3(0.85f, 0.55f, 0.75f) }, // Right tail
            { glm::vec3( 0.3f,  0.70f, -0.1f), glm::vec3(0.90f, 0.70f, 0.80f) }  // Top peak
        };
    }
    else if (type == 1)
    {
        // Type 1: Tall Billowing Cloud (5 overlapping puffs)
        puffs = {
            { glm::vec3( 0.0f,  0.50f,  0.0f), glm::vec3(1.30f, 1.10f, 1.10f) }, // High dome
            { glm::vec3(-0.9f,  0.10f,  0.1f), glm::vec3(1.00f, 0.80f, 0.90f) }, // Left mid
            { glm::vec3( 1.1f,  0.15f, -0.1f), glm::vec3(1.00f, 0.85f, 0.90f) }, // Right mid
            { glm::vec3(-1.6f, -0.20f,  0.0f), glm::vec3(0.80f, 0.55f, 0.70f) }, // Far left
            { glm::vec3( 1.8f, -0.20f,  0.05f), glm::vec3(0.75f, 0.50f, 0.70f) }  // Far right
        };
    }
    else
    {
        // Type 2: Elongated Atmospheric Stratus Cloud (6 puffs)
        puffs = {
            { glm::vec3( 0.0f,  0.00f,  0.0f), glm::vec3(1.50f, 0.60f, 1.00f) }, // Center
            { glm::vec3(-1.4f, -0.05f,  0.1f), glm::vec3(1.20f, 0.55f, 0.85f) }, // Mid left
            { glm::vec3(-2.6f, -0.15f,  0.0f), glm::vec3(0.90f, 0.45f, 0.70f) }, // Outer left
            { glm::vec3( 1.4f,  0.05f, -0.1f), glm::vec3(1.30f, 0.58f, 0.90f) }, // Mid right
            { glm::vec3( 2.7f, -0.12f,  0.05f), glm::vec3(0.85f, 0.40f, 0.65f) }, // Outer right
            { glm::vec3(-0.4f,  0.45f,  0.0f), glm::vec3(0.80f, 0.50f, 0.70f) }  // Small crest
        };
    }
}

void Cloud::update(float dt)
{
    position.x += driftSpeed * dt;

    // Wrap around screen boundaries seamlessly
    if (position.x > maxX)
    {
        position.x = minX;
    }
    else if (position.x < minX)
    {
        position.x = maxX;
    }
}

void Cloud::draw(const Shader& shader, const Sphere& sharedSphere) const
{
    // Base parent transform: Translation to world position and base scale
    glm::mat4 parentTransform = glm::mat4(1.0f);
    parentTransform = glm::translate(parentTransform, position);
    parentTransform = glm::scale(parentTransform, scale);

    // Hierarchical children: Render each puff relative to the cloud parent
    for (const auto& puff : puffs)
    {
        glm::mat4 model = parentTransform;
        model = glm::translate(model, puff.localOffset);
        model = glm::scale(model, puff.localScale);

        shader.setMat4("model", model);
        sharedSphere.draw();
    }
}

void Cloud::setPosition(const glm::vec3& pos)
{
    position = pos;
}

const glm::vec3& Cloud::getPosition() const
{
    return position;
}
