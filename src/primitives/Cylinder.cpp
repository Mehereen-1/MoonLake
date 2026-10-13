#include "primitives/Cylinder.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Cylinder::Cylinder(int segments)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float angleStep = 2.0f * (float)M_PI / segments;
    float radius = 0.5f;

    glm::vec3 defaultColor(0.35f, 0.35f, 0.38f);

    // 1. Bottom Cap (Y = 0.0, normal: 0, -1, 0)
    unsigned int bottomCenterIdx = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), defaultColor });

    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        vertices.push_back({ glm::vec3(x, 0.0f, z), glm::vec3(0.0f, -1.0f, 0.0f), defaultColor });
    }

    for (int i = 0; i < segments; ++i)
    {
        int next = (i + 1) % segments;
        indices.push_back(bottomCenterIdx);
        indices.push_back(bottomCenterIdx + 1 + next);
        indices.push_back(bottomCenterIdx + 1 + i);
    }

    // 2. Top Cap (Y = 1.0, normal: 0, 1, 0)
    unsigned int topCenterIdx = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), defaultColor });

    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        vertices.push_back({ glm::vec3(x, 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), defaultColor });
    }

    for (int i = 0; i < segments; ++i)
    {
        int next = (i + 1) % segments;
        indices.push_back(topCenterIdx);
        indices.push_back(topCenterIdx + 1 + i);
        indices.push_back(topCenterIdx + 1 + next);
    }

    // 3. Side Quads (Y from 0.0 to 1.0, outward radial normal)
    unsigned int sideStartIdx = static_cast<unsigned int>(vertices.size());
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (i % segments) * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        glm::vec3 norm(std::cos(angle), 0.0f, std::sin(angle));

        vertices.push_back({ glm::vec3(x, 0.0f, z), norm, defaultColor }); // bottom
        vertices.push_back({ glm::vec3(x, 1.0f, z), norm, defaultColor }); // top
    }

    for (int i = 0; i < segments; ++i)
    {
        unsigned int b1 = sideStartIdx + i * 2;
        unsigned int t1 = sideStartIdx + i * 2 + 1;
        unsigned int b2 = sideStartIdx + (i + 1) * 2;
        unsigned int t2 = sideStartIdx + (i + 1) * 2 + 1;

        indices.push_back(b1);
        indices.push_back(t1);
        indices.push_back(b2);

        indices.push_back(b2);
        indices.push_back(t1);
        indices.push_back(t2);
    }

    setupMesh(vertices, indices);
}
