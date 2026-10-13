#include "objects/GrassLines.h"
#include "objects/TerrainMesh.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Deterministic pseudo-random generator
static float pseudoRand(int seed)
{
    float f = std::sin((float)seed * 12.9898f + 78.233f) * 43758.5453f;
    return f - std::floor(f);
}

float GrassLines::getShorelineX(float z)
{
    return TerrainMesh::getShorelineX(z);
}

float GrassLines::getGroundElevation(float x, float z, bool& isValidGround)
{
    isValidGround = true;

    // Bounds check across the expanded world
    if (x < -55.0f || x > 55.0f || z < -45.0f || z > 35.0f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Exclude interior of the Wooden Cabin (centered at X = -12.5m, Z = -2.0m)
    if (x > -14.6f && x < -10.4f && z > -4.0f && z < 0.0f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Exclude hot burning campfire center pit (centered at X = -3.8m, Z = 4.5m)
    float fireDistSq = (x - (-3.8f)) * (x - (-3.8f)) + (z - 4.5f) * (z - 4.5f);
    if (fireDistSq < 0.40f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Exclude Lakeside Windmill stone foundation (centered at X = 7.5m, Z = -27.5m)
    if (std::abs(x - 7.5f) < 2.2f && std::abs(z - (-27.5f)) < 2.2f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Exclude interior of Mystical Cave cavern chamber (centered at X = 26.0m, Z = -4.0m)
    float dx = x - 26.0f;
    float dz = z - (-4.0f);
    const float cosRot = 0.529919f;  // cos(-58 deg)
    const float sinRot = -0.848048f; // sin(-58 deg)
    float localX = (cosRot * dx - sinRot * dz) / 1.15f;
    float localZ = (sinRot * dx + cosRot * dz) / 1.15f;
    if (localZ > -14.0f && localZ < 0.2f && std::abs(localX) < 4.8f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Open lake water exclusion (water is at Y = -1.18m)
    float xWest = TerrainMesh::getShorelineX(z);
    float xEast = TerrainMesh::getEastShorelineX(z);
    float zNorth = TerrainMesh::getNorthShorelineZ(x);
    float zSouth = TerrainMesh::getSouthShorelineZ(x);

    if (x > xWest + 0.15f && x < xEast - 0.15f && z > zNorth + 0.15f && z < zSouth - 0.15f)
    {
        isValidGround = false;
        return -2.0f;
    }

    // Compute ground height from the continuous terrain elevation model
    return TerrainMesh::computeElevation(x, z, isValidGround);
}

GrassLines::GrassLines(float defaultHeight, float lineWidth)
    : Mesh(), m_lineWidth(lineWidth)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Palette Definitions
    const glm::vec3 colForestRoot(0.05f, 0.14f, 0.06f);
    const glm::vec3 colForestKnee(0.13f, 0.32f, 0.11f);
    const glm::vec3 colForestTip(0.24f, 0.50f, 0.18f);

    const glm::vec3 colMeadowRoot(0.07f, 0.18f, 0.08f);
    const glm::vec3 colMeadowKnee(0.18f, 0.40f, 0.15f);
    const glm::vec3 colMeadowTip(0.30f, 0.62f, 0.22f);

    const glm::vec3 colReedRoot(0.08f, 0.18f, 0.08f);
    const glm::vec3 colReedKnee(0.22f, 0.38f, 0.14f);
    const glm::vec3 colReedTip(0.36f, 0.54f, 0.20f);

    const glm::vec3 colWaterRoot(0.06f, 0.16f, 0.08f);
    const glm::vec3 colWaterKnee(0.16f, 0.35f, 0.14f);
    const glm::vec3 colWaterTip(0.28f, 0.58f, 0.22f);

    int seed = 1337;

    // Lambda to add a slender, curved grass line blade (3 vertices = 2 line segments)
    auto addGrassBlade = [&](float x, float y, float z, float h, float leanAngle, float leanDist,
                             const glm::vec3& cRoot, const glm::vec3& cKnee, const glm::vec3& cTip)
    {
        float cosA = std::cos(leanAngle);
        float sinA = std::sin(leanAngle);

        glm::vec3 pRoot(x, y, z);
        glm::vec3 pKnee(x + cosA * leanDist * 0.45f, y + h * 0.55f, z + sinA * leanDist * 0.45f);
        glm::vec3 pTip(x + cosA * leanDist, y + h, z + sinA * leanDist);

        glm::vec3 normal(0.0f, 1.0f, 0.0f);

        unsigned int idx = static_cast<unsigned int>(vertices.size());

        vertices.push_back({ pRoot, normal, cRoot });
        vertices.push_back({ pKnee, normal, cKnee });
        vertices.push_back({ pTip,  normal, cTip  });

        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
    };

    // Lambda to add a natural cluster of 3-5 grass line blades
    auto addGrassTuft = [&](float cx, float cy, float cz, int bladeCount, float baseH,
                            const glm::vec3& cRoot, const glm::vec3& cKnee, const glm::vec3& cTip)
    {
        for (int b = 0; b < bladeCount; ++b)
        {
            float rAngle = pseudoRand(seed++) * 6.28318f;
            float rDist = 0.02f + 0.07f * pseudoRand(seed++);
            float rLean = 0.03f + 0.09f * pseudoRand(seed++);
            float rH = baseH * (0.80f + 0.45f * pseudoRand(seed++));

            float bx = cx + std::cos(rAngle) * rDist;
            float bz = cz + std::sin(rAngle) * rDist;

            addGrassBlade(bx, cy, bz, rH, rAngle, rLean, cRoot, cKnee, cTip);
        }
    };

    // -----------------------------------------------------------------------
    // 1. DENSE WESTERN WATERLINE CONCEALMENT (Continuous Curving Ribbon)
    // -----------------------------------------------------------------------
    for (float z = -26.0f; z <= 24.0f; z += 0.12f)
    {
        float xShore = TerrainMesh::getShorelineX(z);
        float r1 = pseudoRand(seed++);
        float r2 = pseudoRand(seed++);

        // Tier A: Waterline edge reeds (roots directly at/in water, Y ~ -1.14m)
        float x1 = xShore + 0.06f + (r1 - 0.5f) * 0.14f;
        float z1 = z + (r2 - 0.5f) * 0.06f;
        bool valid1 = false;
        float y1 = getGroundElevation(x1, z1, valid1);
        if (valid1)
        {
            addGrassTuft(x1, y1, z1, 3, defaultHeight * 1.25f, colWaterRoot, colWaterKnee, colWaterTip);
        }

        // Tier B: Lower bank turf
        float x2 = xShore - 0.25f + (r2 - 0.5f) * 0.16f;
        float z2 = z + (r1 - 0.5f) * 0.06f;
        bool valid2 = false;
        float y2 = getGroundElevation(x2, z2, valid2);
        if (valid2)
        {
            addGrassTuft(x2, y2, z2, 3, defaultHeight * 1.10f, colReedRoot, colReedKnee, colReedTip);
        }

        // Tier C: Mid-bank slope turf
        float x3 = xShore - 0.65f + (r1 - 0.5f) * 0.20f;
        float z3 = z + (r2 - 0.5f) * 0.06f;
        bool valid3 = false;
        float y3 = getGroundElevation(x3, z3, valid3);
        if (valid3)
        {
            addGrassTuft(x3, y3, z3, 3, defaultHeight, colMeadowRoot, colMeadowKnee, colMeadowTip);
        }
    }

    // -----------------------------------------------------------------------
    // 2. COMPLETE WESTERN CAMPSITE MEADOW COVERAGE (Spanning across Lawn)
    // -----------------------------------------------------------------------
    for (float z = -24.0f; z <= 24.0f; z += 0.45f)
    {
        for (float x = -30.0f; x <= 4.0f; x += 0.45f)
        {
            float jx = x + (pseudoRand(seed++) - 0.5f) * 0.38f;
            float jz = z + (pseudoRand(seed++) - 0.5f) * 0.38f;

            bool valid = false;
            float y = getGroundElevation(jx, jz, valid);
            if (!valid) continue;

            const glm::vec3* pRoot = &colMeadowRoot;
            const glm::vec3* pKnee = &colMeadowKnee;
            const glm::vec3* pTip  = &colMeadowTip;

            float h = defaultHeight;

            if (jx < -16.0f)
            {
                pRoot = &colForestRoot; pKnee = &colForestKnee; pTip = &colForestTip;
                h *= 0.88f;
            }
            else if (y < -1.0f)
            {
                pRoot = &colWaterRoot; pKnee = &colWaterKnee; pTip = &colWaterTip;
                h *= 1.15f;
            }

            int bladeCount = 2 + static_cast<int>(pseudoRand(seed++) * 3);
            addGrassTuft(jx, y, jz, bladeCount, h, *pRoot, *pKnee, *pTip);
        }
    }

    // -----------------------------------------------------------------------
    // 3. DENSE EASTERN WATERLINE CONCEALMENT (Continuous Curving Ribbon)
    // -----------------------------------------------------------------------
    for (float ez = -25.0f; ez <= 23.0f; ez += 0.12f)
    {
        float xEast = TerrainMesh::getEastShorelineX(ez);
        float r1 = pseudoRand(seed++);
        float r2 = pseudoRand(seed++);

        // Edge reeds dipping into water
        float ex1 = xEast - 0.06f + (r1 - 0.5f) * 0.14f;
        float ez1 = ez + (r2 - 0.5f) * 0.06f;
        bool valid1 = false;
        float ey1 = getGroundElevation(ex1, ez1, valid1);
        if (valid1)
        {
            addGrassTuft(ex1, ey1, ez1, 3, defaultHeight * 1.25f, colWaterRoot, colWaterKnee, colWaterTip);
        }

        // East bank slope turf
        float ex2 = xEast + 0.30f + (r2 - 0.5f) * 0.16f;
        float ez2 = ez + (r1 - 0.5f) * 0.06f;
        bool valid2 = false;
        float ey2 = getGroundElevation(ex2, ez2, valid2);
        if (valid2)
        {
            addGrassTuft(ex2, ey2, ez2, 3, defaultHeight * 1.10f, colReedRoot, colReedKnee, colReedTip);
        }
    }

    // -----------------------------------------------------------------------
    // 4. EASTERN MEADOW & BLUFF COVERAGE
    // -----------------------------------------------------------------------
    for (float ez = -22.0f; ez <= 22.0f; ez += 0.50f)
    {
        for (float ex = 22.0f; ex <= 38.0f; ex += 0.50f)
        {
            float jx = ex + (pseudoRand(seed++) - 0.5f) * 0.40f;
            float jz = ez + (pseudoRand(seed++) - 0.5f) * 0.40f;
            bool valid = false;
            float y = getGroundElevation(jx, jz, valid);
            if (!valid) continue;

            const glm::vec3* pRoot = &colMeadowRoot;
            const glm::vec3* pKnee = &colMeadowKnee;
            const glm::vec3* pTip  = &colMeadowTip;

            if (y > 0.0f)
            {
                pRoot = &colForestRoot; pKnee = &colForestKnee; pTip = &colForestTip;
            }
            else if (y < -1.0f)
            {
                pRoot = &colWaterRoot; pKnee = &colWaterKnee; pTip = &colWaterTip;
            }

            int bladeCount = 2 + static_cast<int>(pseudoRand(seed++) * 3);
            addGrassTuft(jx, y, jz, bladeCount, defaultHeight, *pRoot, *pKnee, *pTip);
        }
    }

    // -----------------------------------------------------------------------
    // 5. SPECIAL FEATURE CLUSTERS (Campfire, Cabin, Dock, Windmill Knoll, Southern Spit)
    // -----------------------------------------------------------------------
    struct Anchor { glm::vec3 pos; float radius; int tufts; };
    std::vector<Anchor> anchors = {
        // Foreground Southern Shoreline
        { glm::vec3( 1.5f, 0.0f, 12.0f), 3.0f, 40 },
        { glm::vec3(-1.0f, 0.0f, 15.0f), 3.5f, 45 },
        // Dock shore base
        { glm::vec3( 0.2f, 0.0f,  6.8f), 2.0f, 30 },
        { glm::vec3( 0.2f, 0.0f,  9.2f), 2.0f, 30 },
        // Cabin perimeter (X = -12.5m, Z = -2.0m)
        { glm::vec3(-12.5f, 0.0f, -2.0f), 3.5f, 40 },
        { glm::vec3(-10.5f, 0.0f, -0.5f), 2.5f, 25 },
        // Campfire perimeter (X = -3.8m, Z = 4.5m)
        { glm::vec3(-3.8f, 0.0f,  4.5f), 1.8f, 30 },
        { glm::vec3(-5.4f, 0.0f,  5.8f), 1.8f, 25 }, // Chair perimeter
        // Lakeside Windmill Knoll (X = 7.5m, Z = -27.5m)
        { glm::vec3( 7.5f, 0.45f,-27.5f), 4.5f, 45 },
        // Mystical Cave Rocky Approach (X = 24.5m, Z = -4.0m)
        { glm::vec3(24.5f, 0.20f, -4.0f), 3.5f, 35 }
    };

    for (const auto& a : anchors)
    {
        for (int i = 0; i < a.tufts; ++i)
        {
            float ang = pseudoRand(seed++) * 6.28318f;
            float dst = (0.20f + 0.80f * pseudoRand(seed++)) * a.radius;
            float px = a.pos.x + std::cos(ang) * dst;
            float pz = a.pos.z + std::sin(ang) * dst;

            bool valid = false;
            float py = getGroundElevation(px, pz, valid);
            if (!valid) continue;

            addGrassTuft(px, py, pz, 3, defaultHeight * 1.05f, colMeadowRoot, colMeadowKnee, colMeadowTip);
        }
    }

    setupMesh(vertices, indices);
}

void GrassLines::draw() const
{
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(m_lineWidth);

    glBindVertexArray(VAO);
    if (hasIndices)
    {
        glDrawElements(GL_LINES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertexCount));
    }
    glBindVertexArray(0);

    glLineWidth(1.0f);
}
