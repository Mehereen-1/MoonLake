#include "objects/Wildflowers.h"
#include "objects/TouristTrain.h"
#include "objects/TerrainMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float flowerRand(int seed)
{
    float f = std::sin((float)seed * 12.9898f + 78.233f) * 43758.5453f;
    return f - std::floor(f);
}

Wildflowers::Wildflowers()
    : Mesh()
{
    generateFlowerField();
}

void Wildflowers::generateFlowerField()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Palette sampled directly from reference photo:
    // Cosmos Magenta, Soft Pink, Lavender/Lilac, Blossom White, Radiant Violet, Sunshine Buttercup
    const glm::vec3 colMagenta(0.86f, 0.20f, 0.54f);
    const glm::vec3 colPink(0.93f, 0.48f, 0.68f);
    const glm::vec3 colLavender(0.76f, 0.54f, 0.88f);
    const glm::vec3 colWhite(0.97f, 0.95f, 0.92f);
    const glm::vec3 colViolet(0.66f, 0.32f, 0.80f);
    const glm::vec3 colButtercup(0.98f, 0.78f, 0.18f);

    const glm::vec3 petalPalette[6] = {
        colMagenta, colPink, colLavender, colWhite, colViolet, colButtercup
    };

    const glm::vec3 colStemDeep(0.11f, 0.24f, 0.08f);
    const glm::vec3 colStemLight(0.18f, 0.38f, 0.12f);
    const glm::vec3 colPollen(0.98f, 0.84f, 0.16f);

    // Lambda to add a 3D flower plant
    auto addFlower = [&](float rootX, float rootY, float rootZ, float height, float rotY,
                         const glm::vec3& petalCol, float flowerRadius)
    {
        unsigned int baseIndex = (unsigned int)vertices.size();

        float tiltAngle = 0.12f * std::sin(rootX * 3.0f + rootZ * 2.0f);
        float topX = rootX + std::sin(tiltAngle) * height * 0.3f;
        float topY = rootY + height;
        float topZ = rootZ + std::cos(tiltAngle) * height * 0.15f;

        // --- 1. Slender Stem (Crossed ribbon blades) ---
        float stemW = 0.022f;
        float cosR = std::cos(rotY) * stemW;
        float sinR = std::sin(rotY) * stemW;

        // Stem bottom vertices
        vertices.push_back({ glm::vec3(rootX - cosR, rootY, rootZ - sinR), glm::vec3(0, 1, 0), colStemDeep });
        vertices.push_back({ glm::vec3(rootX + cosR, rootY, rootZ + sinR), glm::vec3(0, 1, 0), colStemDeep });
        // Stem top vertices
        vertices.push_back({ glm::vec3(topX - cosR * 0.7f, topY, topZ - sinR * 0.7f), glm::vec3(0, 1, 0), colStemLight });
        vertices.push_back({ glm::vec3(topX + cosR * 0.7f, topY, topZ + sinR * 0.7f), glm::vec3(0, 1, 0), colStemLight });

        // Stem 2 triangles
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 3);

        // --- 2. Blossom Head (Radiating Petal Disc with Golden Center) ---
        unsigned int centerIndex = (unsigned int)vertices.size();
        glm::vec3 blossomCenter(topX, topY + 0.015f, topZ);
        glm::vec3 blossomNorm(0.0f, 0.95f, 0.25f);
        blossomNorm = glm::normalize(blossomNorm);

        // Raised golden center
        vertices.push_back({ blossomCenter, blossomNorm, colPollen });

        const int numPetals = 6;
        for (int p = 0; p < numPetals; ++p)
        {
            float theta = (float)p * (2.0f * (float)M_PI / (float)numPetals) + rotY;
            float rPetal = flowerRadius * (0.85f + 0.15f * std::cos(theta * 3.0f));

            float px = blossomCenter.x + std::cos(theta) * rPetal;
            float pz = blossomCenter.z + std::sin(theta) * rPetal;
            float py = blossomCenter.y + 0.02f * std::sin(theta * 2.0f);

            vertices.push_back({ glm::vec3(px, py, pz), blossomNorm, petalCol });
        }

        for (int p = 0; p < numPetals; ++p)
        {
            unsigned int p1 = centerIndex + 1 + p;
            unsigned int p2 = centerIndex + 1 + ((p + 1) % numPetals);

            indices.push_back(centerIndex);
            indices.push_back(p1);
            indices.push_back(p2);
        }
    };

    bool dummyValid = true;

    // Include TouristTrain distance check to ensure flowers flank the curvy track without obstructing it
    // A. Dense Foreground Drift (Z = 21.5m to 29.5m, X = -28m to +32m)
    for (int i = 0; i < 1100; ++i)
    {
        float rx = flowerRand(i * 7 + 103);
        float rz = flowerRand(i * 13 + 307);
        float x = -28.0f + rx * 60.0f;
        float z = 21.5f + rz * 8.0f;

        // Skip middle of dock / immediate path entrance
        if (x > 9.2f && x < 12.0f && z > 21.8f && z < 23.8f) continue;
        // Keep railway clear
        if (TouristTrain::getDistanceToTrack(x, z) < 1.55f) continue;
        // Keep station platform clear
        if (x > 4.5f && x < 13.5f && z > 27.8f && z < 30.5f) continue;

        float groundY = TerrainMesh::computeElevation(x, z, dummyValid);
        if (groundY < -1.15f) continue; // Skip submerged water

        float h = 0.28f + flowerRand(i * 5 + 17) * 0.24f;
        float rotY = flowerRand(i * 11 + 41) * 360.0f;
        float radius = 0.07f + flowerRand(i * 3 + 29) * 0.035f;

        int palIdx = (int)(flowerRand(i * 19 + 71) * 6.0f) % 6;
        addFlower(x, groundY, z, h, rotY, petalPalette[palIdx], radius);
    }

    // B. Trackside & Mid-Meadow Curving Drifts (Z = 29.5m to 42.0m, X = -34m to +34m)
    for (int i = 0; i < 900; ++i)
    {
        float rx = flowerRand(i * 9 + 501);
        float rz = flowerRand(i * 17 + 811);
        float x = -34.0f + rx * 68.0f;
        float z = 29.5f + rz * 12.5f;

        if (TouristTrain::getDistanceToTrack(x, z) < 1.55f) continue;
        if (x > 4.5f && x < 13.5f && z > 27.8f && z < 30.5f) continue;

        float groundY = TerrainMesh::computeElevation(x, z, dummyValid);
        float h = 0.30f + flowerRand(i * 5 + 67) * 0.26f;
        float rotY = flowerRand(i * 11 + 83) * 360.0f;
        float radius = 0.075f + flowerRand(i * 3 + 43) * 0.035f;

        int palIdx = (int)(flowerRand(i * 23 + 19) * 6.0f) % 6;
        addFlower(x, groundY, z, h, rotY, petalPalette[palIdx], radius);
    }

    // C. High Ridge & Rolling Foothill Drifts (Z = 42.0m to 58.0m, X = -32m to +32m)
    for (int i = 0; i < 700; ++i)
    {
        float rx = flowerRand(i * 11 + 929);
        float rz = flowerRand(i * 19 + 1153);
        float x = -32.0f + rx * 64.0f;
        float z = 42.0f + rz * 16.0f;

        if (TouristTrain::getDistanceToTrack(x, z) < 1.55f) continue;

        float groundY = TerrainMesh::computeElevation(x, z, dummyValid);
        float h = 0.32f + flowerRand(i * 5 + 97) * 0.28f;
        float rotY = flowerRand(i * 7 + 109) * 360.0f;
        float radius = 0.08f + flowerRand(i * 13 + 59) * 0.04f;

        int palIdx = (int)(flowerRand(i * 29 + 31) * 6.0f) % 6;
        addFlower(x, groundY, z, h, rotY, petalPalette[palIdx], radius);
    }

    setupMesh(vertices, indices);
}

void Wildflowers::draw(const Shader& shader) const
{
    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 24.0f);
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("objectColor", glm::vec3(0.0f)); // Enable per-vertex colors
    Mesh::draw();
}
