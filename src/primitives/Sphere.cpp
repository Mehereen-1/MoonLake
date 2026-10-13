#include "primitives/Sphere.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount, const glm::vec3& color)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float lengthInv = 1.0f / radius;
    float sectorStep = 2.0f * (float)M_PI / sectorCount;
    float stackStep = (float)M_PI / stackCount;

    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = (float)M_PI / 2.0f - (float)i * stackStep; // from pi/2 to -pi/2
        float xy = radius * std::cos(stackAngle);
        float y = radius * std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = (float)j * sectorStep; // from 0 to 2pi

            float x = xy * std::cos(sectorAngle);
            float z = xy * std::sin(sectorAngle);

            Vertex v;
            v.position = glm::vec3(x, y, z);
            v.normal = glm::vec3(x * lengthInv, y * lengthInv, z * lengthInv);
            v.color = color;
            vertices.push_back(v);
        }
    }

    // Generate indices for triangles
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    setupMesh(vertices, indices);
}
