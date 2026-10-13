#include "primitives/Branch.h"
#include <vector>
#include <cmath>

Branch::Branch(float length, float radius)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const int segments = 5;
    float angleStep = 6.2831853f / segments;

    glm::vec3 barkColor(0.24f, 0.16f, 0.10f);
    glm::vec3 woodEndColor(0.38f, 0.28f, 0.18f);

    float hl = length * 0.5f;

    // Rings at -hl and +hl
    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float nextAngle = (i + 1) % segments * angleStep;

        float r1 = radius * (0.9f + 0.2f * std::sin(angle * 2.0f));
        float r2 = radius * 0.65f * (0.9f + 0.2f * std::sin(angle * 2.0f)); // Tapered end

        glm::vec3 p0(-hl, r1 * std::cos(angle), r1 * std::sin(angle));
        glm::vec3 p1( hl, r2 * std::cos(angle), r2 * std::sin(angle));
        glm::vec3 p2( hl, r2 * std::cos(nextAngle), r2 * std::sin(nextAngle));
        glm::vec3 p3(-hl, r1 * std::cos(nextAngle), r1 * std::sin(nextAngle));

        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

        unsigned int idx = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ p0, normal, barkColor });
        vertices.push_back({ p1, normal, barkColor });
        vertices.push_back({ p2, normal, barkColor });
        vertices.push_back({ p3, normal, barkColor });

        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);

        indices.push_back(idx);
        indices.push_back(idx + 2);
        indices.push_back(idx + 3);
    }

    setupMesh(vertices, indices);
}
