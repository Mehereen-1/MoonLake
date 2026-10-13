#include "objects/Mountain.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

Mountain::Mountain(float width, float height, float depth)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    // Structure of low-poly alpine mountain ridge:
    // Defined by key ridge peaks, base perimeter points, and foothills
    struct PeakDef {
        glm::vec3 apex;
        float baseRadiusX;
        float baseRadiusZ;
    };

    std::vector<PeakDef> peaks = {
        { glm::vec3(-hw * 0.65f, height * 0.70f, -hd * 0.3f), hw * 0.30f, hd * 0.7f },
        { glm::vec3(-hw * 0.25f, height * 1.00f, -hd * 0.5f), hw * 0.40f, hd * 0.9f }, // Tallest alpine summit
        { glm::vec3( hw * 0.15f, height * 0.82f, -hd * 0.4f), hw * 0.35f, hd * 0.8f },
        { glm::vec3( hw * 0.55f, height * 0.92f, -hd * 0.6f), hw * 0.38f, hd * 0.85f },// Secondary summit
        { glm::vec3( hw * 0.85f, height * 0.58f, -hd * 0.2f), hw * 0.28f, hd * 0.6f }
    };

    glm::vec3 rockColor(0.44f, 0.46f, 0.50f);      // Natural alpine granite rock
    glm::vec3 snowColor(0.95f, 0.97f, 1.00f);      // Frosted alpine snow
    glm::vec3 midRockColor(0.52f, 0.55f, 0.60f);   // Sub-snow ridge scree

    // For each peak, construct an 8-sided faceted pyramid with snow coloring
    for (const auto& peak : peaks)
    {
        const int sides = 7;
        float angleStep = 6.2831853f / sides;

        std::vector<glm::vec3> basePoints;
        for (int i = 0; i < sides; ++i)
        {
            float angle = i * angleStep;
            // Introduce slight organic jitter to radius per side
            float jitter = 0.85f + 0.30f * std::sin(angle * 3.0f + peak.apex.x);
            float bx = peak.apex.x + std::cos(angle) * peak.baseRadiusX * jitter;
            float bz = peak.apex.z + std::sin(angle) * peak.baseRadiusZ * jitter;
            basePoints.push_back(glm::vec3(bx, 0.0f, bz));
        }

        // Apex vertex (Snow)
        Vertex apexVert;
        apexVert.position = peak.apex;
        apexVert.color = snowColor;

        // Build triangular sides
        for (int i = 0; i < sides; ++i)
        {
            int next = (i + 1) % sides;
            glm::vec3 p0 = peak.apex;
            glm::vec3 p1 = basePoints[i];
            glm::vec3 p2 = basePoints[next];

            // Flat-shaded face normal
            glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

            Vertex v0, v1, v2;
            v0.position = p0;
            v0.normal = normal;
            v0.color = snowColor; // Peak is covered in snow

            v1.position = p1;
            v1.normal = normal;
            v1.color = rockColor; // Base is rocky granite

            v2.position = p2;
            v2.normal = normal;
            v2.color = rockColor;

            unsigned int startIdx = static_cast<unsigned int>(vertices.size());
            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            indices.push_back(startIdx);
            indices.push_back(startIdx + 1);
            indices.push_back(startIdx + 2);
        }
    }

    setupMesh(vertices, indices);
}
