#include "primitives/Rock.h"
#include <vector>
#include <cmath>

Rock::Rock(float radius, int seed)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const int sides = 6;
    float angleStep = 6.2831853f / sides;

    glm::vec3 top(0.0f, radius * 0.75f, 0.0f);
    glm::vec3 bottom(0.0f, -radius * 0.35f, 0.0f);

    std::vector<glm::vec3> midRing;
    for (int i = 0; i < sides; ++i)
    {
        float angle = i * angleStep;
        float jitter = 0.85f + 0.30f * std::sin(angle * 2.0f + seed * 1.5f);
        float x = std::cos(angle) * radius * jitter;
        float y = 0.15f * std::sin(angle * 3.0f + seed);
        float z = std::sin(angle) * radius * jitter * 0.85f;
        midRing.push_back(glm::vec3(x, y, z));
    }

    glm::vec3 rockColor(0.22f, 0.24f, 0.27f);

    // Build top and bottom triangular facets
    for (int i = 0; i < sides; ++i)
    {
        int next = (i + 1) % sides;

        // Top triangle (top, midRing[i], midRing[next])
        glm::vec3 nTop = glm::normalize(glm::cross(midRing[i] - top, midRing[next] - top));
        unsigned int startIdx = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ top, nTop, rockColor });
        vertices.push_back({ midRing[i], nTop, rockColor });
        vertices.push_back({ midRing[next], nTop, rockColor });
        indices.push_back(startIdx);
        indices.push_back(startIdx + 1);
        indices.push_back(startIdx + 2);

        // Bottom triangle (midRing[next], midRing[i], bottom)
        glm::vec3 nBot = glm::normalize(glm::cross(midRing[next] - bottom, midRing[i] - bottom));
        startIdx = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ midRing[next], nBot, rockColor * 0.85f });
        vertices.push_back({ midRing[i], nBot, rockColor * 0.85f });
        vertices.push_back({ bottom, nBot, rockColor * 0.85f });
        indices.push_back(startIdx);
        indices.push_back(startIdx + 1);
        indices.push_back(startIdx + 2);
    }

    setupMesh(vertices, indices);
}
