#include "primitives/GrassTuft.h"
#include <vector>
#include <cmath>

GrassTuft::GrassTuft(float scale)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 grassBaseColor(0.12f, 0.28f, 0.14f);
    glm::vec3 grassTipColor(0.24f, 0.52f, 0.22f);

    const int blades = 4;
    float angleStep = 3.14159265f / blades;

    for (int i = 0; i < blades; ++i)
    {
        float angle = i * angleStep;
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);

        float halfW = 0.15f * scale;
        float h = (0.50f + 0.15f * std::sin(angle * 3.0f)) * scale;

        // Base points
        glm::vec3 p0(-sinA * halfW, 0.0f, cosA * halfW);
        glm::vec3 p1( sinA * halfW, 0.0f, -cosA * halfW);
        // Apex tip (slightly leaning)
        glm::vec3 tip(cosA * 0.12f * scale, h, sinA * 0.12f * scale);

        // Blade normal
        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, tip - p0));

        unsigned int idx = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ p0, normal, grassBaseColor });
        vertices.push_back({ p1, normal, grassBaseColor });
        vertices.push_back({ tip, normal, grassTipColor });

        // Double-sided rendering so blades are visible from both sides
        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);

        indices.push_back(idx + 2);
        indices.push_back(idx + 1);
        indices.push_back(idx);
    }

    setupMesh(vertices, indices);
}
