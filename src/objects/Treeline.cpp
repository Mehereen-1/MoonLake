#include "objects/Treeline.h"
#include "objects/TerrainMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

static float treelineRand(int seed)
{
    float f = std::sin((float)seed * 12.9898f + 78.233f) * 43758.5453f;
    return f - std::floor(f);
}

Treeline::Treeline()
{
    initForest();
}

void Treeline::initForest()
{
    trees.clear();
    trees.reserve(90);

    bool dummyValid = true;

    // Palette variants for deep natural forest greens
    const glm::vec3 colSpruce1(0.06f, 0.14f, 0.08f);
    const glm::vec3 colSpruce2(0.08f, 0.18f, 0.10f);
    const glm::vec3 colPineDeep(0.05f, 0.12f, 0.06f);
    const glm::vec3 colForestCanopy(0.10f, 0.22f, 0.11f);
    const glm::vec3 colHighlandPine(0.07f, 0.16f, 0.09f);

    const glm::vec3 foliagePalette[5] = {
        colSpruce1, colSpruce2, colPineDeep, colForestCanopy, colHighlandPine
    };

    // 1. Front Treeline Tier (Z = 50m to 64m) - Frames the rolling meadow
    for (float x = -60.0f; x <= 62.0f; x += 4.8f)
    {
        int seed = (int)(x * 17.0f + 101.0f);
        float xOff = (treelineRand(seed) - 0.5f) * 3.5f;
        float z = 54.0f + (treelineRand(seed + 1) - 0.5f) * 8.0f;
        float finalX = x + xOff;
        float y = TerrainMesh::computeElevation(finalX, z, dummyValid) - 0.10f;
        float scale = 1.9f + treelineRand(seed + 2) * 0.9f;
        float rotY = treelineRand(seed + 3) * 360.0f;
        int type = (seed % 3);
        glm::vec3 col = foliagePalette[std::abs(seed) % 5];

        trees.push_back({ glm::vec3(finalX, y, z), scale, rotY, type, col });
    }

    // 2. Mid-Distant Forest Ridge Tier (Z = 68m to 82m) - Dense canopy backing the horizon
    for (float x = -64.0f; x <= 64.0f; x += 4.2f)
    {
        int seed = (int)(x * 23.0f + 509.0f);
        float xOff = (treelineRand(seed) - 0.5f) * 3.2f;
        float z = 74.0f + (treelineRand(seed + 1) - 0.5f) * 9.0f;
        float finalX = x + xOff;
        float y = TerrainMesh::computeElevation(finalX, z, dummyValid) - 0.12f;
        float scale = 2.4f + treelineRand(seed + 2) * 1.1f;
        float rotY = treelineRand(seed + 3) * 360.0f;
        int type = (seed % 3 == 0) ? 0 : 1;
        glm::vec3 col = foliagePalette[(std::abs(seed) + 2) % 5];

        trees.push_back({ glm::vec3(finalX, y, z), scale, rotY, type, col });
    }

    // 3. Skyline Ridge Forest Tier (Z = 88m to 106m) - Towering trees crowning the mountain horizon
    for (float x = -68.0f; x <= 68.0f; x += 3.8f)
    {
        int seed = (int)(x * 31.0f + 983.0f);
        float xOff = (treelineRand(seed) - 0.5f) * 3.0f;
        float z = 96.0f + (treelineRand(seed + 1) - 0.5f) * 10.0f;
        float finalX = x + xOff;
        float y = TerrainMesh::computeElevation(finalX, z, dummyValid) - 0.15f;
        float scale = 3.0f + treelineRand(seed + 2) * 1.4f;
        float rotY = treelineRand(seed + 3) * 360.0f;
        int type = (seed % 2);
        glm::vec3 col = foliagePalette[(std::abs(seed) + 4) % 5];

        trees.push_back({ glm::vec3(finalX, y, z), scale, rotY, type, col });
    }
}

void Treeline::draw(const Shader& shader,
                    const Cube& unitCube,
                    const Cylinder& unitCylinder,
                    const Sphere& unitSphere,
                    const Pyramid& unitPyramid) const
{
    const glm::vec3 barkDark(0.24f, 0.15f, 0.09f);

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 10.0f);

    for (const auto& t : trees)
    {
        glm::mat4 root(1.0f);
        root = glm::translate(root, t.pos);
        root = glm::rotate(root, glm::radians(t.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        root = glm::scale(root, glm::vec3(t.scale));

        if (t.treeType == 2)
        {
            // Broadleaf Canopy Tree
            // Trunk
            glm::mat4 trunk = glm::translate(root, glm::vec3(0.0f, 1.6f, 0.0f));
            trunk = glm::scale(trunk, glm::vec3(0.42f, 3.2f, 0.42f));
            shader.setMat4("model", trunk);
            shader.setVec3("objectColor", barkDark);
            unitCylinder.draw();

            // Clustered Foliage Spheres
            const glm::vec3 domeOffsets[4] = {
                glm::vec3( 0.0f,  3.6f,  0.0f),
                glm::vec3( 0.6f,  3.1f,  0.4f),
                glm::vec3(-0.5f,  3.0f, -0.3f),
                glm::vec3( 0.1f,  4.2f, -0.1f)
            };
            const glm::vec3 domeScales[4] = {
                glm::vec3(2.4f, 2.0f, 2.4f),
                glm::vec3(1.9f, 1.6f, 1.8f),
                glm::vec3(1.8f, 1.5f, 1.9f),
                glm::vec3(1.6f, 1.4f, 1.6f)
            };

            for (int i = 0; i < 4; ++i)
            {
                glm::mat4 leafMat = glm::translate(root, domeOffsets[i]);
                leafMat = glm::scale(leafMat, domeScales[i]);
                shader.setMat4("model", leafMat);
                shader.setVec3("objectColor", t.foliageColor * (0.92f + 0.06f * (float)i));
                unitSphere.draw();
            }
        }
        else
        {
            // Alpine Pine / Spruce (Layered Pyramids)
            // Slender Pine Trunk
            glm::mat4 trunk = glm::translate(root, glm::vec3(0.0f, 1.5f, 0.0f));
            trunk = glm::scale(trunk, glm::vec3(0.30f, 3.0f, 0.30f));
            shader.setMat4("model", trunk);
            shader.setVec3("objectColor", barkDark);
            unitCube.draw();

            // 4 Layered tiers
            const float yTiers[4] = { 1.8f, 3.0f, 4.1f, 5.1f };
            const float scaleTiers[4] = { 3.2f, 2.5f, 1.8f, 1.1f };
            const float rotTiers[4] = { 0.0f, 28.0f, -18.0f, 35.0f };

            for (int i = 0; i < 4; ++i)
            {
                glm::mat4 tierMat = glm::translate(root, glm::vec3(0.0f, yTiers[i], 0.0f));
                tierMat = glm::rotate(tierMat, glm::radians(rotTiers[i]), glm::vec3(0.0f, 1.0f, 0.0f));
                tierMat = glm::scale(tierMat, glm::vec3(scaleTiers[i], 2.0f, scaleTiers[i]));
                shader.setMat4("model", tierMat);
                shader.setVec3("objectColor", t.foliageColor * (0.88f + 0.08f * (float)i));
                unitPyramid.draw();
            }
        }
    }
}
