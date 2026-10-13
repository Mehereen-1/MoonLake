#include "objects/TerrainMesh.h"
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper smoothstep function
static inline float smoothstep(float edge0, float edge1, float x)
{
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float TerrainMesh::getShorelineX(float z)
{
    // Natural western shoreline with coves, promontories, and beaches
    if (z > 14.0f)
    {
        // Southern beach curving gently outward into southern reed bay
        float t = (z - 14.0f) / 12.0f;
        return 0.5f + 1.8f * std::sin(std::clamp(t, 0.0f, 1.0f) * (float)M_PI * 0.5f) + 0.12f * (z - 14.0f);
    }
    else if (z > 6.0f)
    {
        // Waterfront & dock area: gentle beach at X ~ 0.5m
        float t = (z - 6.0f) / 8.0f;
        return 0.2f + 0.6f * std::sin(t * (float)M_PI);
    }
    else if (z > 0.0f)
    {
        // Campfire Cove: gently curves inward toward campsite at Z = 3.5m
        float t = z / 6.0f;
        return -0.4f - 0.75f * std::sin(t * (float)M_PI);
    }
    else if (z > -12.0f)
    {
        // Cabin grassy promontory: curves outward into lake at Z = -6.0m
        float t = (z + 12.0f) / 12.0f;
        return 0.3f + 0.85f * std::sin(t * (float)M_PI);
    }
    else if (z > -26.0f)
    {
        // Northern lake narrows: sinuous shoreline
        float t = (z + 26.0f) / 14.0f;
        return 0.1f - 0.65f * std::sin(t * (float)M_PI);
    }
    else
    {
        // Sweeping curve meeting the northern bay
        float t = std::clamp((-(z + 26.0f)) / 6.0f, 0.0f, 2.0f);
        return 0.8f + 0.9f * t;
    }
}

float TerrainMesh::getEastShorelineX(float z)
{
    // Eastern lake shoreline contour across from the campsite
    if (z > 12.0f)
    {
        // Southern eastern bank
        float t = (z - 12.0f) / 14.0f;
        return 23.5f - 1.5f * std::sin(std::clamp(t, 0.0f, 1.0f) * (float)M_PI * 0.5f);
    }
    else if (z > -10.0f)
    {
        // In front of the Mystical Cave: rugged stone promontory jutting into the lake
        float t = (z + 10.0f) / 22.0f;
        return 24.5f - 2.2f * std::sin(t * (float)M_PI);
    }
    else if (z > -26.0f)
    {
        // North-east lake curve
        float t = (z + 26.0f) / 16.0f;
        return 23.0f + 1.2f * std::sin(t * (float)M_PI);
    }
    else
    {
        // Northern bay eastern edge
        float t = std::clamp((-(z + 26.0f)) / 6.0f, 0.0f, 2.0f);
        return 21.0f - 0.8f * t;
    }
}

float TerrainMesh::getNorthShorelineZ(float x)
{
    // Northern crescent boundary of the lake in front of Windmill Bluff
    float t = std::clamp((x - 0.0f) / 22.0f, 0.0f, 1.0f);
    return -26.5f + 2.5f * std::sin(t * (float)M_PI);
}

float TerrainMesh::getSouthShorelineZ(float x)
{
    // Southern reed bay boundary of the lake
    float t = std::clamp((x - 0.0f) / 23.0f, 0.0f, 1.0f);
    return 24.5f - 2.2f * std::sin(t * (float)M_PI);
}

float TerrainMesh::computeElevation(float x, float z, bool& isValidLand)
{
    isValidLand = true;

    const float waterY = -1.18f;
    const float lawnY = -0.75f;

    float xWest = getShorelineX(z);
    float xEast = getEastShorelineX(z);
    float zNorth = getNorthShorelineZ(x);
    float zSouth = getSouthShorelineZ(x);

    bool inLakeBounds = (x > xWest && x < xEast && z > zNorth && z < zSouth);

    if (inLakeBounds)
    {
        // Inside the lake basin: smoothly submerged lakebed
        float dWest = x - xWest;
        float dEast = xEast - x;
        float dNorth = z - zNorth;
        float dSouth = zSouth - z;
        float dEdge = std::min(std::min(dWest, dEast), std::min(dNorth, dSouth));

        // Center lakebed drops down to -2.4m
        float centerBedY = -2.35f - 0.20f * std::sin(x * 0.12f) * std::cos(z * 0.10f);
        float s = smoothstep(0.0f, 3.8f, dEdge);
        return waterY * (1.0f - s) + centerBedY * s;
    }

    // --- On dry land / surrounding landscape ---
    // 1. WESTERN CAMPSITE & FOOTHILLS (x <= xWest)
    if (x <= xWest && z >= zNorth && z <= zSouth)
    {
        float dShore = xWest - x;
        if (dShore < 2.5f)
        {
            // Sinuous gentle bank rising from water edge to campsite lawn
            float s = smoothstep(0.0f, 2.5f, dShore);
            return waterY * (1.0f - s) + lawnY * s;
        }

        // Campsite lawn and western foothills
        if (x >= -20.0f)
        {
            // Flat comfortable forest campsite meadow
            return lawnY + 0.04f * std::sin(x * 0.35f) * std::cos(z * 0.25f);
        }
        else
        {
            // Rolling western foothills rising toward mountain flank
            float dist = -20.0f - x;
            float hillRise = 0.0035f * dist * dist + 0.05f * dist;
            float rolls = 0.45f * std::sin(z * 0.12f) * std::cos(x * 0.15f);
            return lawnY + hillRise + rolls;
        }
    }

    // 2. EASTERN BLUFFS & MOUNTAIN TERRACE (x >= xEast)
    if (x >= xEast && z >= zNorth && z <= zSouth)
    {
        float dShore = x - xEast;
        if (dShore < 2.5f)
        {
            float s = smoothstep(0.0f, 2.5f, dShore);
            return waterY * (1.0f - s) + lawnY * s;
        }

        // Eastern shoreline meadow (smooth natural lawn surrounding the tunnel)
        float hillRise = 0.0f;
        if (x > 48.0f)
        {
            float dist = x - 48.0f;
            hillRise = 0.004f * dist * dist + 0.06f * dist;
        }

        float rolls = 0.12f * std::sin(z * 0.15f) * std::cos(x * 0.12f);
        return lawnY + hillRise + rolls;
    }

    // 3. NORTHERN ROLLING BLUFF & WINDMILL KNOLL (z <= zNorth)
    if (z <= zNorth)
    {
        float dShore = zNorth - z;
        if (dShore < 2.5f && x >= xWest && x <= xEast)
        {
            float s = smoothstep(0.0f, 2.5f, dShore);
            return waterY * (1.0f - s) + (lawnY + 0.10f) * s;
        }

        // Elevated knoll for the Lakeside Windmill (centered at X ~ 7.5m, Z ~ -27.5m)
        float dxM = x - 7.5f;
        float dzM = z - (-27.5f);
        float windmillKnoll = 1.25f * std::exp(-0.012f * (dxM * dxM + dzM * dzM));

        // Northern mountain foothills
        float hillRise = 0.0f;
        if (z < -34.0f)
        {
            float dist = -34.0f - z;
            hillRise = 0.0038f * dist * dist + 0.06f * dist;
        }

        // Flanking side terrain matching west/east rises
        float sideRise = 0.0f;
        if (x < -20.0f) sideRise = 0.0035f * (-20.0f - x) * (-20.0f - x);
        else if (x > 32.0f) sideRise = 0.0035f * (x - 32.0f) * (x - 32.0f);

        float rolls = 0.35f * std::sin(x * 0.14f) * std::cos(z * 0.12f);
        return lawnY + windmillKnoll + hillRise + sideRise + rolls;
    }

    // 4. SOUTHERN EXPANDED MEADOW & DISTANT ROLLING RIDGE (z >= zSouth)
    float dShore = z - zSouth;
    if (dShore < 2.5f && x >= xWest && x <= xEast)
    {
        float s = smoothstep(0.0f, 2.5f, dShore);
        return waterY * (1.0f - s) + lawnY * s;
    }

    // Meadow terrain gently sloping away from the lake (Z = 22 to 38m)
    float meadowSlope = 0.016f * (z - 22.0f);
    float meadowSwell = 0.14f * std::sin((x - 10.0f) * 0.12f) * std::cos((z - 28.0f) * 0.10f);

    // Distant rolling hills and majestic southern ridge (Z > 36m extending to 130m)
    // Rises smoothly to ~10 - 18 meters in the far distance, completely blocking the edge of the world
    float distantRidge = 0.0f;
    if (z > 36.0f)
    {
        float dzFar = z - 36.0f;
        distantRidge = 0.0014f * dzFar * dzFar + 0.080f * dzFar
                     + 0.85f * std::sin(x * 0.075f + 1.2f) * std::cos(dzFar * 0.05f)
                     + 0.50f * std::sin((x + 15.0f) * 0.12f);
    }

    float sideRise = 0.0f;
    if (x < -22.0f) sideRise = 0.0032f * (-22.0f - x) * (-22.0f - x);
    else if (x > 28.0f) sideRise = 0.0032f * (x - 28.0f) * (x - 28.0f);

    float rolls = 0.22f * std::sin(x * 0.15f) * std::cos(z * 0.12f);
    return lawnY + meadowSlope + meadowSwell + distantRidge + sideRise + rolls;
}

TerrainMesh::TerrainMesh(float minX, float maxX, float minZ, float maxZ, int gridX, int gridZ)
    : Mesh()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float dx = (maxX - minX) / gridX;
    float dz = (maxZ - minZ) / gridZ;

    const glm::vec3 colGrass(0.32f, 0.58f, 0.22f);     // Vibrant natural forest lawn green
    const glm::vec3 colShore(0.50f, 0.46f, 0.35f);     // Warm golden shoreline sand / turf
    const glm::vec3 colSubmerged(0.18f, 0.26f, 0.22f); // Submerged lakebed silt
    const glm::vec3 colRock(0.42f, 0.44f, 0.48f);      // Granite cliff & rocky knoll
    const glm::vec3 colHighland(0.36f, 0.54f, 0.24f);  // Sunny alpine hillside meadow

    // 1. Generate Vertices with smooth analytical heights, normals & color blending
    for (int iz = 0; iz <= gridZ; ++iz)
    {
        float z = minZ + iz * dz;
        for (int ix = 0; ix <= gridX; ++ix)
        {
            float x = minX + ix * dx;

            bool valid = true;
            float y = computeElevation(x, z, valid);

            // Compute surface normal via central difference for silky smooth shading
            bool vDummy = true;
            float delta = 0.25f;
            float yL = computeElevation(x - delta, z, vDummy);
            float yR = computeElevation(x + delta, z, vDummy);
            float yD = computeElevation(x, z - delta, vDummy);
            float yU = computeElevation(x, z + delta, vDummy);

            glm::vec3 vx(2.0f * delta, yR - yL, 0.0f);
            glm::vec3 vz(0.0f, yU - yD, 2.0f * delta);
            glm::vec3 norm = glm::normalize(glm::cross(vz, vx));

            // Per-vertex color blending based on elevation, slope and region
            glm::vec3 col;
            if (y < -1.18f)
            {
                // Submerged lakebed
                float blend = std::clamp((-1.18f - y) / 0.8f, 0.0f, 1.0f);
                col = glm::mix(colShore, colSubmerged, blend);
            }
            else if (y < -0.70f)
            {
                // Water edge to lawn
                float blend = std::clamp((y - (-1.18f)) / 0.48f, 0.0f, 1.0f);
                col = glm::mix(colShore, colGrass, blend);
            }
            else if (y < 1.0f)
            {
                // Forest lawn to meadow
                float blend = std::clamp((y - (-0.70f)) / 1.7f, 0.0f, 1.0f);
                col = glm::mix(colGrass, colHighland, blend);
            }
            else
            {
                // Highland grass to mountain rock
                float blend = std::clamp((y - 1.0f) / 3.5f, 0.0f, 1.0f);
                col = glm::mix(colHighland, colRock, blend);
            }

            // Southern meadow & distant rolling ridge color blending (lush natural green grass, warm loam & alpine highland)
            if (z > 22.0f && y >= -1.15f)
            {
                float meadowWeight = smoothstep(22.0f, 26.5f, z);
                const glm::vec3 colMeadowGreen(0.32f, 0.58f, 0.22f);    // Lush natural lawn & meadow
                const glm::vec3 colMeadowVibrant(0.40f, 0.68f, 0.26f);  // Sunny clover & field grass
                float toneMix = 0.5f + 0.5f * std::sin(x * 0.35f + z * 0.28f);
                glm::vec3 meadowGround = glm::mix(colMeadowGreen, colMeadowVibrant, toneMix);

                if (z > 45.0f)
                {
                    float ridgeBlend = std::clamp((z - 45.0f) / 45.0f, 0.0f, 1.0f);
                    meadowGround = glm::mix(meadowGround, colHighland, ridgeBlend * 0.70f);
                }

                col = glm::mix(col, meadowGround, meadowWeight * 0.90f);
            }

            // Steep slope rock shading
            if (norm.y < 0.80f)
            {
                float slopeRock = std::clamp((0.80f - norm.y) / 0.35f, 0.0f, 1.0f);
                col = glm::mix(col, colRock, slopeRock * 0.75f);
            }

            vertices.push_back({ glm::vec3(x, y, z), norm, col });
        }
    }

    // 2. Generate Triangles
    for (int iz = 0; iz < gridZ; ++iz)
    {
        for (int ix = 0; ix < gridX; ++ix)
        {
            unsigned int row1 = iz * (gridX + 1);
            unsigned int row2 = (iz + 1) * (gridX + 1);

            indices.push_back(row1 + ix);
            indices.push_back(row2 + ix);
            indices.push_back(row1 + ix + 1);

            indices.push_back(row1 + ix + 1);
            indices.push_back(row2 + ix);
            indices.push_back(row2 + ix + 1);
        }
    }

    setupMesh(vertices, indices);
}

// Legacy constructor for backward compatibility
TerrainMesh::TerrainMesh(bool isWestSide)
    : TerrainMesh(-65.0f, 65.0f, -55.0f, 45.0f, 130, 100)
{
}

