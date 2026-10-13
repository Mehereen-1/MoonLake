#include "objects/Water.h"
#include <vector>

Water::Water(float width, float depth, int gridX, int gridZ)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float dx = width / gridX;
    float dz = depth / gridZ;
    float startX = -width * 0.5f;
    float startZ = -depth * 0.5f;

    glm::vec3 waterColor(0.04f, 0.10f, 0.18f); // Deep reflective lake water
    glm::vec3 upNormal(0.0f, 1.0f, 0.0f);

    for (int z = 0; z <= gridZ; ++z)
    {
        for (int x = 0; x <= gridX; ++x)
        {
            Vertex v;
            v.position = glm::vec3(startX + x * dx, 0.0f, startZ + z * dz);
            v.normal = upNormal;
            v.color = waterColor;
            vertices.push_back(v);
        }
    }

    for (int z = 0; z < gridZ; ++z)
    {
        for (int x = 0; x < gridX; ++x)
        {
            unsigned int row1 = z * (gridX + 1);
            unsigned int row2 = (z + 1) * (gridX + 1);

            // Triangle 1
            indices.push_back(row1 + x);
            indices.push_back(row2 + x);
            indices.push_back(row1 + x + 1);

            // Triangle 2
            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);
            indices.push_back(row2 + x + 1);
        }
    }

    setupMesh(vertices, indices);
}
