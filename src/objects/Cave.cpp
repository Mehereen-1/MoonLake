#include "objects/Cave.h"
#include "MotionSystem.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Fast deterministic float hash [0.0, 1.0] for organic uneven stone variations
static inline float stoneHash(unsigned int seed, int salt)
{
    unsigned int h = seed * 1664525u + (unsigned int)salt * 1013904223u;
    h ^= (h >> 13);
    h *= 1274126177u;
    return (float)(h & 0xFFFF) / 65535.0f;
}

Cave::Cave(const glm::vec3& pos, float rotY, float scale)
    : position(pos), rotationY(rotY), scaleFactor(scale)
{
    // Procedurally generate the complete authentic cobblestone tunnel mesh
    buildCobblestoneTunnel();
}

void Cave::setPosition(const glm::vec3& pos) { position = pos; }
void Cave::setRotation(float rotY)           { rotationY = rotY; }
void Cave::setScale(float scale)             { scaleFactor = scale; }

const glm::vec3& Cave::getPosition() const   { return position; }
float Cave::getRotation() const              { return rotationY; }
float Cave::getScale() const                 { return scaleFactor; }

glm::mat4 Cave::getRootMatrix() const
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scaleFactor));
    return model;
}

void Cave::toggleTorchLit()
{
    torchLit = !torchLit;
}

const char* Cave::getTorchBrightnessName() const
{
    switch (brightnessLevel)
    {
        case 0: return "DIM (0.5x)";
        case 1: return "NORMAL (1.0x)";
        case 2: return "BRIGHT (1.6x)";
        case 3: return "BLAZING (2.2x)";
        default: return "NORMAL (1.0x)";
    }
}

void Cave::cycleTorchBrightness()
{
    brightnessLevel = (brightnessLevel + 1) % 4;
    switch (brightnessLevel)
    {
        case 0: torchBrightness = 0.5f; break;
        case 1: torchBrightness = 1.0f; break;
        case 2: torchBrightness = 1.6f; break;
        case 3: torchBrightness = 2.2f; break;
        default: torchBrightness = 1.0f; break;
    }
}

glm::vec3 Cave::getInteriorLightPos() const
{
    return glm::vec3(getRootMatrix() * glm::vec4(0.0f, 1.8f, -2.5f, 1.0f));
}

// ---------------------------------------------------------------------------
// Uneven, Organic Tunnel Cross-Section & Meander Model
// ---------------------------------------------------------------------------
struct TunnelCrossSection {
    float xc;        // Centerline X offset
    float wLeft;     // Half-width to left wall
    float wRight;    // Half-width to right wall
    float springY;   // Springline height Y (where vertical wall meets arch)
    float crownExtra;// Arch apex height boost
};

static inline TunnelCrossSection getCrossSection(float z, float zStart = 0.55f)
{
    float s = std::max(0.0f, zStart - z);
    // Smooth cubic blending from entrance doorway (entrance remains straight & aligned with exterior portal)
    float blend = std::min(1.0f, s / 1.6f);
    blend = blend * blend * (3.0f - 2.0f * blend);

    // Organic meandering path (gentle undulating S-curves through the bedrock)
    float xc = (0.42f * std::sin(s * 0.17f) + 0.24f * std::sin(s * 0.38f + 0.5f) - 0.10f * std::cos(s * 0.62f)) * blend;

    // Asymmetric wall widths: left and right walls wander independently (breaks cylindrical symmetry)
    float varLeft  = 0.13f * std::sin(s * 0.23f + 0.6f) + 0.07f * std::cos(s * 0.55f);
    float varRight = 0.12f * std::cos(s * 0.27f + 1.1f) - 0.06f * std::sin(s * 0.49f);
    float wLeft  = 1.55f * (1.0f + varLeft  * blend);
    float wRight = 1.55f * (1.0f + varRight * blend);

    // Springline height variations (vertical walls vary in height along depth)
    float varSpring = 0.11f * std::sin(s * 0.28f + 0.8f) + 0.06f * std::cos(s * 0.65f);
    float springY = 1.35f * (1.0f + varSpring * blend);

    // Arch crown height variation (natural cavern vault height fluctuations)
    float crownExtra = 0.18f * std::sin(s * 0.31f + 1.4f) * blend;

    return { xc, wLeft, wRight, springY, crownExtra };
}

static inline void getArchPoint(const TunnelCrossSection& cs, float theta, float s,
                                glm::vec3& outPos, glm::vec3& outNormal, glm::vec3& outTangent)
{
    float t = theta / 3.14159265f;
    float rBase = glm::mix(cs.wRight, cs.wLeft, t);

    float sinT = std::sin(theta);
    float blend = std::min(1.0f, s / 1.6f);
    blend = blend * blend * (3.0f - 2.0f * blend);

    // Uneven harmonics (gothic/pointed peak, asymmetrical shoulders)
    float harmonic = (0.08f * std::sin(3.0f * theta + s * 0.42f)
                    + 0.05f * std::sin(5.0f * theta + 1.1f)) * sinT * blend;
    float crownBoost = cs.crownExtra * sinT;

    float R = rBase + harmonic;

    float px = cs.xc + R * std::cos(theta);
    float py = cs.springY + R * std::sin(theta) + crownBoost;
    outPos = glm::vec3(px, py, 0.0f);

    // Tangent along arch perimeter (counter-clockwise from right theta=0 to left theta=pi)
    glm::vec3 T = glm::normalize(glm::vec3(std::sin(theta), -std::cos(theta), 0.0f));

    // Inward normal (T x B where B = (0, 0, 1)):
    glm::vec3 N = glm::vec3(-std::cos(theta), -std::sin(theta), 0.0f);

    outNormal = N;
    outTangent = T;
}

// ---------------------------------------------------------------------------
// Mathematical Cave Pathway Definition (P0 -> P1 -> P2 -> P3)
// ---------------------------------------------------------------------------
std::vector<CavePathSection> Cave::getPathSections() const
{
    const float zStart = 0.55f;
    auto cs1 = getCrossSection(-4.5f, zStart);
    auto cs2 = getCrossSection(-14.0f, zStart);
    auto cs3 = getCrossSection(-23.5f, zStart);

    return {
        // P0: Entrance stone arch threshold
        { glm::vec3(0.0f, 0.0f,  1.2f), 3.1f, 3.1f, 3.0f, 0.8f },
        // P1: Entering the curving cobblestone corridor
        { glm::vec3(cs1.xc, 0.0f, -4.5f), cs1.wLeft + cs1.wRight, cs1.springY * 2.0f, 9.0f, 0.8f },
        // P2: Deep meandering cobblestone passage
        { glm::vec3(cs2.xc, 0.0f, -14.0f), cs2.wLeft + cs2.wRight, cs2.springY * 2.0f, 10.0f, 0.8f },
        // P3: Approaching the rustic dead-end stone wall
        { glm::vec3(cs3.xc, 0.0f, -23.5f), cs3.wLeft + cs3.wRight, cs3.springY * 2.0f, 9.0f, 0.8f }
    };
}

// ---------------------------------------------------------------------------
// Guided Cave Tour Camera Path Evaluation (C(t) = (1 - t)Pi + tP_{i+1})
// ---------------------------------------------------------------------------
glm::vec3 Cave::getTourWorldPosition(float tourProgress) const
{
    const float zStart = 0.55f;
    auto cs1 = getCrossSection(-4.5f, zStart);
    auto cs2 = getCrossSection(-14.0f, zStart);
    auto cs3 = getCrossSection(-23.5f, zStart);

    // 4 key waypoints at eye level down the curving tunnel centerline
    const glm::vec3 waypoints[4] = {
        glm::vec3(0.0f, 1.35f,  2.5f),           // P0: Outside entrance arch looking into the cobblestone tunnel
        glm::vec3(cs1.xc, 1.35f, -4.5f),         // P1: Inside the meandering cobblestone corridor
        glm::vec3(cs2.xc, 1.35f, -14.0f),        // P2: Moving deeper down the uneven stone passage
        glm::vec3(cs3.xc + 0.35f, 1.15f, -20.6f) // P3: Smoothly framing the glowing Cave Treasure Sanctum & Gold Coins
    };

    float p = glm::clamp(tourProgress, 0.0f, 3.0f);
    int idx = (int)std::floor(p);
    if (idx >= 3)
    {
        return glm::vec3(getRootMatrix() * glm::vec4(waypoints[3], 1.0f));
    }
    float t = p - (float)idx;
    float smoothT = 0.5f * (1.0f - std::cos(t * 3.14159265f));
    glm::vec3 localPos = glm::mix(waypoints[idx], waypoints[idx + 1], smoothT);

    return glm::vec3(getRootMatrix() * glm::vec4(localPos, 1.0f));
}

glm::vec3 Cave::getTourWorldLookTarget(float tourProgress) const
{
    const float zStart = 0.55f;
    auto lookCs0 = getCrossSection(-3.0f, zStart);
    auto lookCs1 = getCrossSection(-12.0f, zStart);
    auto lookCs2 = getCrossSection(-20.0f, zStart);
    auto lookCs3 = getCrossSection(-22.6f, zStart);

    // Looking forward along the curving tunnel centerline towards the treasure and wall
    const glm::vec3 lookTargets[4] = {
        glm::vec3(lookCs0.xc, 1.35f, -3.0f),
        glm::vec3(lookCs1.xc, 1.35f, -12.0f),
        glm::vec3(lookCs2.xc, 1.35f, -20.0f),
        glm::vec3(lookCs3.xc - 0.65f, 0.38f, -22.6f) // P3: Looking directly at the open treasure chest and illuminating gold coins
    };

    float p = glm::clamp(tourProgress, 0.0f, 3.0f);
    int idx = (int)std::floor(p);
    if (idx >= 3)
    {
        return glm::vec3(getRootMatrix() * glm::vec4(lookTargets[3], 1.0f));
    }
    float t = p - (float)idx;
    float smoothT = 0.5f * (1.0f - std::cos(t * 3.14159265f));
    glm::vec3 localLook = glm::mix(lookTargets[idx], lookTargets[idx + 1], smoothT);

    return glm::vec3(getRootMatrix() * glm::vec4(localLook, 1.0f));
}

// ---------------------------------------------------------------------------
// Localized Glowing Treasure Hoard & Gold Coins Position & Lighting Getters
// ---------------------------------------------------------------------------
glm::vec3 Cave::getTreasureWorldPos() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    glm::vec3 localPos(cs.xc - 0.55f, 0.35f, -22.4f);
    return glm::vec3(getRootMatrix() * glm::vec4(localPos, 1.0f));
}

glm::vec3 Cave::getTreasureLightColor(float time) const
{
    // Warm flickering golden & fiery luminescence matching blazing torches
    float pulse = 0.92f + 0.08f * std::sin(time * 6.8f) + 0.05f * std::cos(time * 11.4f);
    return glm::vec3(1.0f, 0.74f, 0.26f) * pulse;
}

float Cave::getTreasureLightIntensity(float time) const
{
    // Dynamic organic flame flicker
    float flicker = 0.16f * std::sin(time * 14.5f) + 0.10f * std::cos(time * 24.2f);
    return 2.30f + flicker;
}

glm::vec3 Cave::getTreasureViewCameraPos() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    // Camera positioned at eye level slightly back and to the right in the tunnel passage
    glm::vec3 localCam(cs.xc + 0.40f, 0.95f, -20.6f);
    return glm::vec3(getRootMatrix() * glm::vec4(localCam, 1.0f));
}

glm::vec3 Cave::getTreasureViewLookTarget() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    // Looking directly at the open chest and the shimmering coins
    glm::vec3 localLook(cs.xc - 0.65f, 0.38f, -22.6f);
    return glm::vec3(getRootMatrix() * glm::vec4(localLook, 1.0f));
}

glm::vec3 Cave::getHangingOrbWorldPos() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    const float xChest = cs.xc - 0.72f;
    const float yChest = 0.0f;
    const float rotYChest = 24.0f;

    glm::mat4 chestMat = getRootMatrix();
    chestMat = glm::translate(chestMat, glm::vec3(xChest, yChest, zChest));
    chestMat = glm::rotate(chestMat, glm::radians(rotYChest), glm::vec3(0.0f, 1.0f, 0.0f));

    return glm::vec3(chestMat * glm::vec4(0.08f, 1.95f, 0.12f, 1.0f));
}

glm::vec3 Cave::getChestMinBounds() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    const float xChest = cs.xc - 0.72f;
    const float yChest = 0.0f;
    const float rotYChest = 24.0f;

    glm::mat4 chestMat = getRootMatrix();
    chestMat = glm::translate(chestMat, glm::vec3(xChest, yChest, zChest));
    chestMat = glm::rotate(chestMat, glm::radians(rotYChest), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 p0 = glm::vec3(chestMat * glm::vec4(-0.55f, 0.0f, -0.35f, 1.0f));
    glm::vec3 p1 = glm::vec3(chestMat * glm::vec4( 0.55f, 0.45f,  0.35f, 1.0f));
    return glm::min(p0, p1);
}

glm::vec3 Cave::getChestMaxBounds() const
{
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    const float xChest = cs.xc - 0.72f;
    const float yChest = 0.0f;
    const float rotYChest = 24.0f;

    glm::mat4 chestMat = getRootMatrix();
    chestMat = glm::translate(chestMat, glm::vec3(xChest, yChest, zChest));
    chestMat = glm::rotate(chestMat, glm::radians(rotYChest), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 p0 = glm::vec3(chestMat * glm::vec4(-0.55f, 0.0f, -0.35f, 1.0f));
    glm::vec3 p1 = glm::vec3(chestMat * glm::vec4( 0.55f, 0.45f,  0.35f, 1.0f));
    return glm::max(p0, p1);
}


// ---------------------------------------------------------------------------
// Flaming Torch Stand Position & Dynamic Light Position
// ---------------------------------------------------------------------------
glm::vec3 Cave::getTorchStandWorldPos() const
{
    // Torch stand is mounted outside the cave entrance on the right side of the grass meadow
    glm::vec3 localStand(1.35f, 0.0f, 1.35f);
    return glm::vec3(getRootMatrix() * glm::vec4(localStand, 1.0f));
}

glm::vec3 Cave::getActiveTorchLightPos(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camRight, const glm::vec3& camUp) const
{
    if (playerHasTorch)
    {
        // Held in front of camera
        return camPos + camRight * 0.24f + camFront * 0.52f - camUp * 0.10f;
    }
    else
    {
        // Burning at the flame head of the stand outside the cave entrance
        return glm::vec3(getRootMatrix() * glm::vec4(1.35f, 1.32f, 1.35f, 1.0f));
    }
}

// ---------------------------------------------------------------------------
// Procedural Cobblestone Tunnel Mesh Generator
// - Uneven, rustic hand-laid stone blocks with varied sizes, protrusion depths,
//   and angular tilts matching the authentic low-poly reference image
// - Natural simple grass ground bed and scattered blooming wildflowers
//   (NO stone paths on the ground!)
// - Dressed stone entrance portal voussoirs (NO green moss)
// - Weathered stone exterior vault (NO green moss, NO attached mountain)
// ---------------------------------------------------------------------------
void Cave::buildCobblestoneTunnel()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    auto addTriangle = [&](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                           const glm::vec3& col0, const glm::vec3& col1, const glm::vec3& col2)
    {
        glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
        float len = glm::length(normal);
        if (len > 1e-6f) normal /= len;
        else normal = glm::vec3(0.0f, 1.0f, 0.0f);

        unsigned int idx = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ p0, normal, col0 });
        vertices.push_back({ p1, normal, col1 });
        vertices.push_back({ p2, normal, col2 });

        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
    };

    auto addQuad = [&](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
                       const glm::vec3& c0, const glm::vec3& c1, const glm::vec3& c2, const glm::vec3& c3)
    {
        addTriangle(p0, p1, p2, c0, c1, c2);
        addTriangle(p0, p2, p3, c0, c2, c3);
    };

    // Helper to generate a single 3D uneven, rustic low-poly cobblestone block
    auto addCobblestone = [&](const glm::vec3& P_in, const glm::vec3& N_in, const glm::vec3& T_in,
                              float w_in, float l_in, float h_in, const glm::vec3& col, unsigned int seed)
    {
        // Tangent space on the wall surface: B is perpendicular to normal N and tangent T
        glm::vec3 B_in = glm::normalize(glm::cross(N_in, T_in));

        // 1. Organic size variations per stone (making stones noticeably uneven)
        float randW = 0.82f + 0.36f * stoneHash(seed, 1); // Width variation (0.82 to 1.18)
        float randL = 0.80f + 0.40f * stoneHash(seed, 2); // Length variation (0.80 to 1.20)
        float randH = 0.65f + 0.70f * stoneHash(seed, 3); // Protrusion variation (0.65 to 1.35: uneven protrusion)
        float w = w_in * randW;
        float l = l_in * randL;
        float h = h_in * randH;

        float hw = w * 0.5f;
        float hl = l * 0.5f;

        // 2. Position jitter along the wall surface (breaking straight seams)
        float jitterT = (stoneHash(seed, 4) - 0.5f) * 0.045f;
        float jitterB = (stoneHash(seed, 5) - 0.5f) * 0.040f;
        float jitterN = (stoneHash(seed, 6) - 0.5f) * 0.025f;
        glm::vec3 P = P_in + jitterT * T_in + jitterB * B_in + jitterN * N_in;

        // 3. Angular tilt / orientation jitter (uneven stone face angles catching light uniquely)
        float tiltAngleT = (stoneHash(seed, 7) - 0.5f) * 0.16f;
        float tiltAngleB = (stoneHash(seed, 8) - 0.5f) * 0.14f;
        float rollAngle  = (stoneHash(seed, 9) - 0.5f) * 0.15f;

        glm::vec3 T = glm::normalize(T_in + tiltAngleT * N_in + rollAngle * B_in);
        glm::vec3 B = glm::normalize(B_in + tiltAngleB * N_in - rollAngle * T_in);
        glm::vec3 N = glm::normalize(glm::cross(T, B));

        // 4. Asymmetric bevels per stone (rough-hewn uneven stone borders)
        float bevelW0 = std::min(0.065f, hw * (0.22f + 0.18f * stoneHash(seed, 10)));
        float bevelW1 = std::min(0.065f, hw * (0.22f + 0.18f * stoneHash(seed, 11)));
        float bevelL0 = std::min(0.065f, hl * (0.22f + 0.18f * stoneHash(seed, 12)));
        float bevelL1 = std::min(0.065f, hl * (0.22f + 0.18f * stoneHash(seed, 13)));

        float iw_left  = hw - bevelW0;
        float iw_right = hw - bevelW1;
        float il_bot   = hl - bevelL0;
        float il_top   = hl - bevelL1;

        // Base vertices on the mortar wall surface (recessed slightly)
        glm::vec3 b0 = P - 0.015f * N - hw * T - hl * B;
        glm::vec3 b1 = P - 0.015f * N + hw * T - hl * B;
        glm::vec3 b2 = P - 0.015f * N + hw * T + hl * B;
        glm::vec3 b3 = P - 0.015f * N - hw * T + hl * B;

        // Front face vertices (raised inward along N by h)
        glm::vec3 f0 = P + h * N - iw_left  * T - il_bot * B;
        glm::vec3 f1 = P + h * N + iw_right * T - il_bot * B;
        glm::vec3 f2 = P + h * N + iw_right * T + il_top * B;
        glm::vec3 f3 = P + h * N - iw_left  * T + il_top * B;

        // 5. Off-center apex vertex (giving natural uneven crowned pillow shape)
        float apexOffT = (stoneHash(seed, 14) - 0.5f) * 0.05f;
        float apexOffB = (stoneHash(seed, 15) - 0.5f) * 0.05f;
        float apexExtraH = 0.018f + 0.016f * stoneHash(seed, 16);
        glm::vec3 apex = P + (h + apexExtraH) * N + apexOffT * T + apexOffB * B;

        glm::vec3 cTop = col;
        glm::vec3 cBevel1 = col * 0.80f; // Bottom shadow
        glm::vec3 cBevel2 = col * 0.90f; // Right slope
        glm::vec3 cBevel3 = col * 0.85f; // Top slope
        glm::vec3 cBevel4 = col * 0.85f; // Left slope

        // Front crowned face (4 triangles radiating from apex)
        addTriangle(apex, f1, f2, cTop, cTop, cTop);
        addTriangle(apex, f2, f3, cTop, cTop, cTop);
        addTriangle(apex, f3, f0, cTop, cTop, cTop);
        addTriangle(apex, f0, f1, cTop, cTop, cTop);

        // Four beveled perimeter slope quads connecting base to front face
        addQuad(b0, b1, f1, f0, cBevel1, cBevel1, cTop, cTop); // Bottom slope
        addQuad(b1, b2, f2, f1, cBevel2, cBevel2, cTop, cTop); // Right slope
        addQuad(b2, b3, f3, f2, cBevel3, cBevel3, cTop, cTop); // Top slope
        addQuad(b3, b0, f0, f3, cBevel4, cBevel4, cTop, cTop); // Left slope
    };

    // Authentic stone color palette sampled directly from the reference image
    static const glm::vec3 stonePalette[8] = {
        glm::vec3(0.68f, 0.44f, 0.24f), // Warm golden ochre
        glm::vec3(0.58f, 0.36f, 0.18f), // Deep terracotta amber
        glm::vec3(0.72f, 0.48f, 0.26f), // Bright sandstone
        glm::vec3(0.52f, 0.32f, 0.16f), // Rustic clay brown
        glm::vec3(0.64f, 0.42f, 0.22f), // Tawny stone
        glm::vec3(0.55f, 0.35f, 0.19f), // Medium amber
        glm::vec3(0.70f, 0.46f, 0.25f), // Golden tan
        glm::vec3(0.48f, 0.30f, 0.15f)  // Dark earth brown
    };

    const float zStart = 0.55f;                  // Entrance portal Z
    const float zEnd = -26.0f;                   // Extended deep end Z (much longer tunnel)
    const float ringStepZ = 0.42f;
    const int numRingsZ = static_cast<int>(std::ceil((zStart - zEnd) / ringStepZ));

    const int stonesLeft = 4;
    const int stonesArch = 11;
    const int stonesRight = 4;
    const int totalStonesPerRing = stonesLeft + stonesArch + stonesRight;

    // --- 1. GENERATE UNEVEN COBBLESTONE MASONRY COURSES ---
    for (int ringIdx = 0; ringIdx < numRingsZ; ++ringIdx)
    {
        float z = zStart - (float)ringIdx * ringStepZ;
        bool isStaggered = (ringIdx % 2 == 1); // Alternating running bond pattern
        auto cs = getCrossSection(z, zStart);
        float s = std::max(0.0f, zStart - z);

        float stoneLenZ = ringStepZ * 0.88f;

        for (int sIdx = 0; sIdx < totalStonesPerRing; ++sIdx)
        {
            glm::vec3 P(0.0f);
            glm::vec3 N(0.0f);
            glm::vec3 T(0.0f);
            float stoneW = 0.36f;
            float stoneH = 0.088f;

            if (sIdx < stonesLeft)
            {
                // Left Vertical Wall (varying in position and height)
                float stepY = (cs.springY - 0.08f) / (float)stonesLeft;
                float baseY = 0.08f + ((float)sIdx + 0.5f) * stepY;
                if (isStaggered) baseY += stepY * 0.45f;
                baseY = glm::clamp(baseY, 0.06f, cs.springY);

                P = glm::vec3(cs.xc - cs.wLeft, baseY, z);
                N = glm::vec3(1.0f, 0.0f, 0.0f);  // Inward normal (+X)
                T = glm::vec3(0.0f, 1.0f, 0.0f);  // Tangent (+Y)
                stoneW = stepY * 0.88f;
            }
            else if (sIdx < stonesLeft + stonesArch)
            {
                // Non-circular, Uneven Arch Ceiling
                int archIdx = sIdx - stonesLeft;
                float dTheta = (float)M_PI / (float)stonesArch;
                float theta = (float)M_PI - ((float)archIdx + 0.5f) * dTheta;
                if (isStaggered) theta -= dTheta * 0.5f;
                theta = glm::clamp(theta, 0.04f, (float)M_PI - 0.04f);

                getArchPoint(cs, theta, s, P, N, T);
                P.z = z;

                float avgR = (cs.wLeft + cs.wRight) * 0.5f;
                stoneW = (avgR * dTheta) * 0.88f;
            }
            else
            {
                // Right Vertical Wall (varying in position and height)
                int rIdx = sIdx - (stonesLeft + stonesArch);
                float stepY = (cs.springY - 0.08f) / (float)stonesRight;
                float baseY = cs.springY - ((float)rIdx + 0.5f) * stepY;
                if (isStaggered) baseY += stepY * 0.45f;
                baseY = glm::clamp(baseY, 0.06f, cs.springY);

                P = glm::vec3(cs.xc + cs.wRight, baseY, z);
                N = glm::vec3(-1.0f, 0.0f, 0.0f); // Inward normal (-X)
                T = glm::vec3(0.0f, -1.0f, 0.0f); // Tangent (-Y)
                stoneW = stepY * 0.88f;
            }

            // Pseudo-random deterministic color selection per stone
            unsigned int seed = (unsigned int)(ringIdx * 131 + sIdx * 79 + 23);
            int palIdx = seed % 8;
            float jitter = 0.93f + 0.14f * (float)(seed % 100) / 100.0f;
            glm::vec3 stoneCol = stonePalette[palIdx] * jitter;

            // Generate uneven stone block
            addCobblestone(P, N, T, stoneW, stoneLenZ, stoneH, stoneCol, seed);
        }
    }

    // --- 2. RECESSED DARK MORTAR BACKING WALL ---
    // Continuous dark grout layer behind the stones along the meandering, uneven tunnel
    const glm::vec3 mortarBase(0.095f, 0.080f, 0.070f);
    const int mortarArchSegments = 16;
    float mDTheta = (float)M_PI / (float)mortarArchSegments;

    for (int ringIdx = 0; ringIdx < numRingsZ; ++ringIdx)
    {
        float z0 = zStart - (float)ringIdx * ringStepZ + ringStepZ * 0.5f;
        float z1 = z0 - ringStepZ;
        auto cs0 = getCrossSection(z0, zStart);
        auto cs1 = getCrossSection(z1, zStart);
        float s0 = std::max(0.0f, zStart - z0);
        float s1 = std::max(0.0f, zStart - z1);

        glm::vec3 mCol = mortarBase;

        // Left mortar wall
        addQuad(glm::vec3(cs0.xc - cs0.wLeft - 0.012f, 0.0f, z0),
                glm::vec3(cs0.xc - cs0.wLeft - 0.012f, cs0.springY, z0),
                glm::vec3(cs1.xc - cs1.wLeft - 0.012f, cs1.springY, z1),
                glm::vec3(cs1.xc - cs1.wLeft - 0.012f, 0.0f, z1),
                mCol, mCol, mCol, mCol);

        // Right mortar wall
        addQuad(glm::vec3(cs1.xc + cs1.wRight + 0.012f, 0.0f, z1),
                glm::vec3(cs1.xc + cs1.wRight + 0.012f, cs1.springY, z1),
                glm::vec3(cs0.xc + cs0.wRight + 0.012f, cs0.springY, z0),
                glm::vec3(cs0.xc + cs0.wRight + 0.012f, 0.0f, z0),
                mCol, mCol, mCol, mCol);

        // Arch ceiling mortar (winding arranged for inward-facing normals)
        for (int a = 0; a < mortarArchSegments; ++a)
        {
            float t0 = (float)M_PI - (float)a * mDTheta;
            float t1 = (float)M_PI - (float)(a + 1) * mDTheta;

            glm::vec3 p00, n00, tg00;
            glm::vec3 p01, n01, tg01;
            glm::vec3 p10, n10, tg10;
            glm::vec3 p11, n11, tg11;

            getArchPoint(cs0, t0, s0, p00, n00, tg00);
            getArchPoint(cs0, t1, s0, p01, n01, tg01);
            getArchPoint(cs1, t0, s1, p10, n10, tg10);
            getArchPoint(cs1, t1, s1, p11, n11, tg11);

            // Push slightly outward behind stones along normal
            p00 = p00 - 0.012f * n00; p00.z = z0;
            p01 = p01 - 0.012f * n01; p01.z = z0;
            p10 = p10 - 0.012f * n10; p10.z = z1;
            p11 = p11 - 0.012f * n11; p11.z = z1;

            addQuad(p00, p10, p11, p01, mCol, mCol, mCol, mCol);
        }
    }

    // --- 3. NATURAL SIMPLE GRASS GROUND (NO STONE PATH!) ---
    // Continuous natural grass lawn ground covering the entire floor from entrance to the back wall
    const glm::vec3 colGrassBase(0.24f, 0.38f, 0.15f); // Natural green lawn
    const glm::vec3 colGrassDeep(0.18f, 0.30f, 0.12f); // Shaded grass

    const int grassGridX = 6;
    const int grassGridZ = numRingsZ;

    for (int gz = 0; gz < grassGridZ; ++gz)
    {
        float z0 = zStart - (float)gz * ringStepZ + ringStepZ * 0.5f;
        float z1 = z0 - ringStepZ;
        auto cs0 = getCrossSection(z0, zStart);
        auto cs1 = getCrossSection(z1, zStart);

        float dAvg = std::max(0.0f, -(z0 + z1) * 0.5f);
        float gFade = 0.08f + 0.92f * std::exp(-dAvg / 6.0f);
        glm::vec3 gCol = glm::mix(colGrassDeep, colGrassBase, gFade);

        float xLeft0  = cs0.xc - cs0.wLeft;
        float xRight0 = cs0.xc + cs0.wRight;
        float xLeft1  = cs1.xc - cs1.wLeft;
        float xRight1 = cs1.xc + cs1.wRight;

        for (int gx = 0; gx < grassGridX; ++gx)
        {
            float u0 = (float)gx / (float)grassGridX;
            float u1 = (float)(gx + 1) / (float)grassGridX;

            glm::vec3 p00(glm::mix(xLeft0, xRight0, u0), 0.01f, z0);
            glm::vec3 p01(glm::mix(xLeft0, xRight0, u1), 0.01f, z0);
            glm::vec3 p11(glm::mix(xLeft1, xRight1, u1), 0.01f, z1);
            glm::vec3 p10(glm::mix(xLeft1, xRight1, u0), 0.01f, z1);

            addQuad(p00, p01, p11, p10, gCol, gCol, gCol, gCol);
        }
    }

    // Extended natural meadow grass in front of the entrance (Z = 0.5 to 2.8m)
    for (int gz = 0; gz < 5; ++gz)
    {
        float z0 = zStart + (float)gz * 0.45f;
        float z1 = z0 + 0.45f;
        auto cs0 = getCrossSection(z0, zStart);
        auto cs1 = getCrossSection(z1, zStart);
        float xLeft0  = cs0.xc - cs0.wLeft;
        float xRight0 = cs0.xc + cs0.wRight;
        float xLeft1  = cs1.xc - cs1.wLeft;
        float xRight1 = cs1.xc + cs1.wRight;

        for (int gx = 0; gx < grassGridX; ++gx)
        {
            float u0 = (float)gx / (float)grassGridX;
            float u1 = (float)(gx + 1) / (float)grassGridX;

            glm::vec3 p00(glm::mix(xLeft0, xRight0, u0), 0.01f, z0);
            glm::vec3 p01(glm::mix(xLeft0, xRight0, u1), 0.01f, z0);
            glm::vec3 p11(glm::mix(xLeft1, xRight1, u1), 0.01f, z1);
            glm::vec3 p10(glm::mix(xLeft1, xRight1, u0), 0.01f, z1);

            addQuad(p00, p01, p11, p10, colGrassBase, colGrassBase, colGrassBase, colGrassBase);
        }
    }

    // --- 4. SIMPLE 3D GRASS TUFTS & BLOOMING WILDFLOWERS SCATTERED ON THE GROUND ---
    auto addSimpleGrass = [&](float gx, float gy, float gz, float scale, unsigned int seed)
    {
        const int blades = 3;
        float dAngle = (float)M_PI / (float)blades;
        glm::vec3 cRoot(0.12f, 0.24f, 0.08f);
        glm::vec3 cTip(0.26f, 0.48f, 0.16f);

        for (int b = 0; b < blades; ++b)
        {
            float ang = (float)b * dAngle + (float)(seed % 31) * 0.2f;
            float cosA = std::cos(ang);
            float sinA = std::sin(ang);
            float halfBladeW = 0.032f * scale;
            float h = (0.20f + 0.08f * std::sin((float)b * 2.1f)) * scale;

            glm::vec3 p0(gx - sinA * halfBladeW, gy, gz + cosA * halfBladeW);
            glm::vec3 p1(gx + sinA * halfBladeW, gy, gz - cosA * halfBladeW);
            glm::vec3 tip(gx + cosA * 0.035f * scale, gy + h, gz + sinA * 0.035f * scale);

            addTriangle(p0, p1, tip, cRoot, cRoot, cTip);
            addTriangle(p1, p0, tip, cRoot, cRoot, cTip); // Double-sided
        }
    };

    auto addWildflower = [&](float fx, float fy, float fz, float h, const glm::vec3& petalCol, unsigned int seed)
    {
        // Slender green stem
        glm::vec3 sBase(fx, fy, fz);
        float tiltX = 0.025f * std::sin((float)seed * 1.7f);
        float tiltZ = 0.025f * std::cos((float)seed * 2.3f);
        glm::vec3 sTop(fx + tiltX, fy + h, fz + tiltZ);
        glm::vec3 cStem(0.14f, 0.30f, 0.10f);

        float sw = 0.012f;
        addQuad(glm::vec3(sBase.x - sw, sBase.y, sBase.z), glm::vec3(sBase.x + sw, sBase.y, sBase.z),
                glm::vec3(sTop.x + sw, sTop.y, sTop.z), glm::vec3(sTop.x - sw, sTop.y, sTop.z),
                cStem, cStem, cStem, cStem);
        addQuad(glm::vec3(sBase.x, sBase.y, sBase.z - sw), glm::vec3(sBase.x, sBase.y, sBase.z + sw),
                glm::vec3(sTop.x, sTop.y, sTop.z + sw), glm::vec3(sTop.x, sTop.y, sTop.z - sw),
                cStem, cStem, cStem, cStem);

        // Radiant blossom head
        glm::vec3 cPollen(0.98f, 0.82f, 0.18f); // Golden center
        glm::vec3 bCenter = sTop + glm::vec3(0.0f, 0.008f, 0.0f);

        const int numPetals = 6;
        float pRadius = 0.065f;
        float dPetalAngle = 2.0f * (float)M_PI / (float)numPetals;
        float baseRot = (float)(seed % 50) * 0.12f;

        for (int p = 0; p < numPetals; ++p)
        {
            float a0 = (float)p * dPetalAngle + baseRot;
            float a1 = (float)(p + 1) * dPetalAngle + baseRot;

            glm::vec3 p0(bCenter.x + std::cos(a0) * pRadius, bCenter.y + 0.008f, bCenter.z + std::sin(a0) * pRadius);
            glm::vec3 p1(bCenter.x + std::cos(a1) * pRadius, bCenter.y + 0.008f, bCenter.z + std::sin(a1) * pRadius);

            addTriangle(bCenter, p0, p1, cPollen, petalCol, petalCol);
            addTriangle(bCenter, p1, p0, cPollen, petalCol, petalCol);
        }
    };

    // Flower petal colors
    const glm::vec3 fPink(0.93f, 0.48f, 0.68f);
    const glm::vec3 fYellow(0.98f, 0.80f, 0.20f);
    const glm::vec3 fWhite(0.96f, 0.95f, 0.92f);
    const glm::vec3 fLavender(0.78f, 0.58f, 0.88f);
    const glm::vec3 fViolet(0.68f, 0.35f, 0.82f);
    const glm::vec3 fColors[5] = { fPink, fYellow, fWhite, fLavender, fViolet };

    // Scatter simple grass tufts and blooming wildflowers near the entrance
    struct FloraInstance {
        float x, z, scale, h;
        int colIdx;
        bool isFlower;
    };

    const FloraInstance floraData[] = {
        // Outside entrance on the meadow lawn
        { -0.85f,  1.85f, 1.10f, 0.22f, 0, true },
        { -0.35f,  2.20f, 1.00f, 0.24f, 1, true },
        {  0.45f,  1.90f, 1.15f, 0.20f, 2, true },
        {  0.95f,  2.10f, 1.05f, 0.23f, 3, true },
        { -1.15f,  1.30f, 0.95f, 0.19f, 4, true },
        {  1.10f,  1.40f, 1.10f, 0.22f, 0, true },
        { -0.55f,  1.10f, 0.90f, 0.18f, 1, true },
        {  0.65f,  1.05f, 1.00f, 0.21f, 2, true },
        // Grass tufts outside
        { -0.65f,  2.05f, 1.10f, 0.0f, 0, false },
        {  0.15f,  2.30f, 1.25f, 0.0f, 0, false },
        {  0.80f,  1.75f, 1.05f, 0.0f, 0, false },
        { -1.05f,  1.60f, 1.15f, 0.0f, 0, false },
        {  1.25f,  1.85f, 1.00f, 0.0f, 0, false },
        { -0.20f,  1.35f, 1.20f, 0.0f, 0, false },
        {  0.35f,  1.45f, 1.10f, 0.0f, 0, false },

        // At the entrance portal threshold (Z = 0.0 to 0.6m)
        { -1.10f,  0.45f, 1.00f, 0.20f, 3, true },
        {  1.05f,  0.40f, 1.05f, 0.22f, 4, true },
        { -0.40f,  0.50f, 0.95f, 0.19f, 1, true },
        {  0.50f,  0.55f, 1.00f, 0.21f, 0, true },
        { -1.25f,  0.30f, 1.10f, 0.0f, 0, false },
        {  1.20f,  0.35f, 1.15f, 0.0f, 0, false },
        { -0.75f,  0.25f, 1.05f, 0.0f, 0, false },
        {  0.75f,  0.20f, 1.20f, 0.0f, 0, false },

        // Inside the front of the cave on the grass ground (Z = -0.5 to -3.5m)
        { -1.15f, -0.60f, 0.95f, 0.18f, 2, true },
        {  1.10f, -0.80f, 1.00f, 0.20f, 3, true },
        { -0.85f, -1.40f, 0.90f, 0.17f, 0, true },
        {  0.90f, -1.60f, 0.95f, 0.18f, 1, true },
        { -1.05f, -2.40f, 0.85f, 0.16f, 4, true },
        {  1.00f, -2.60f, 0.90f, 0.17f, 2, true },
        // Grass tufts inside
        { -1.20f, -0.40f, 1.10f, 0.0f, 0, false },
        {  1.15f, -0.50f, 1.05f, 0.0f, 0, false },
        { -0.60f, -0.90f, 1.15f, 0.0f, 0, false },
        {  0.55f, -1.10f, 1.10f, 0.0f, 0, false },
        { -1.10f, -1.80f, 1.00f, 0.0f, 0, false },
        {  1.05f, -2.00f, 1.05f, 0.0f, 0, false },
        { -0.70f, -2.80f, 0.95f, 0.0f, 0, false },
        {  0.65f, -3.10f, 0.90f, 0.0f, 0, false }
    };

    for (size_t i = 0; i < sizeof(floraData) / sizeof(floraData[0]); ++i)
    {
        const auto& f = floraData[i];
        unsigned int fSeed = (unsigned int)(i * 73 + 19);
        float alignX = f.x + getCrossSection(f.z, zStart).xc;
        if (f.isFlower)
        {
            addWildflower(alignX, 0.015f, f.z, f.h, fColors[f.colIdx], fSeed);
        }
        else
        {
            addSimpleGrass(alignX, 0.015f, f.z, f.scale, fSeed);
        }
    }

    // --- 5. RUSTIC DRESSED STONE ENTRANCE ARCH PORTAL (NO GREEN MOSS) ---
    // Authentic carved masonry voussoirs framing the entrance at Z = zStart
    const float portalZ0 = zStart;
    const float portalZ1 = zStart + 0.35f;
    const float halfW0 = 1.55f;
    const float springY0 = 1.35f;
    const float portalOuterR = halfW0 + 0.55f;
    const int portalVoussoirs = 11;
    const float pDTheta = (float)M_PI / (float)portalVoussoirs;
    const glm::vec3 portalStoneLight(0.48f, 0.42f, 0.35f);
    const glm::vec3 portalStoneDark(0.38f, 0.33f, 0.27f);

    // Left and right entrance stone plinths
    addQuad(glm::vec3(-portalOuterR, 0.0f, portalZ1), glm::vec3(-halfW0, 0.0f, portalZ1),
            glm::vec3(-halfW0, springY0, portalZ1), glm::vec3(-portalOuterR, springY0, portalZ1),
            portalStoneLight, portalStoneDark, portalStoneLight, portalStoneDark);
    addQuad(glm::vec3(halfW0, 0.0f, portalZ1), glm::vec3(portalOuterR, 0.0f, portalZ1),
            glm::vec3(portalOuterR, springY0, portalZ1), glm::vec3(halfW0, springY0, portalZ1),
            portalStoneDark, portalStoneLight, portalStoneDark, portalStoneLight);

    // Outer and side faces of plinths
    addQuad(glm::vec3(-portalOuterR, 0.0f, portalZ0), glm::vec3(-portalOuterR, 0.0f, portalZ1),
            glm::vec3(-portalOuterR, springY0, portalZ1), glm::vec3(-portalOuterR, springY0, portalZ0),
            portalStoneDark, portalStoneLight, portalStoneLight, portalStoneDark);
    addQuad(glm::vec3(portalOuterR, 0.0f, portalZ1), glm::vec3(portalOuterR, 0.0f, portalZ0),
            glm::vec3(portalOuterR, springY0, portalZ0), glm::vec3(portalOuterR, springY0, portalZ1),
            portalStoneLight, portalStoneDark, portalStoneDark, portalStoneLight);

    // Radial voussoirs around the arch
    for (int v = 0; v < portalVoussoirs; ++v)
    {
        float t0 = (float)M_PI - (float)v * pDTheta;
        float t1 = (float)M_PI - (float)(v + 1) * pDTheta;
        glm::vec3 vCol = (v % 2 == 0) ? portalStoneLight : portalStoneDark;

        glm::vec3 pIn0(halfW0 * std::cos(t0), springY0 + halfW0 * std::sin(t0), portalZ1);
        glm::vec3 pIn1(halfW0 * std::cos(t1), springY0 + halfW0 * std::sin(t1), portalZ1);
        glm::vec3 pOut1(portalOuterR * std::cos(t1), springY0 + portalOuterR * std::sin(t1), portalZ1);
        glm::vec3 pOut0(portalOuterR * std::cos(t0), springY0 + portalOuterR * std::sin(t0), portalZ1);

        // Front face of voussoir facing the outside
        addQuad(pIn0, pIn1, pOut1, pOut0, vCol, vCol, vCol, vCol);

        // Outer rim face
        glm::vec3 oR0(portalOuterR * std::cos(t0), springY0 + portalOuterR * std::sin(t0), portalZ0);
        glm::vec3 oR1(portalOuterR * std::cos(t1), springY0 + portalOuterR * std::sin(t1), portalZ0);
        addQuad(pOut0, pOut1, oR1, oR0, vCol * 0.9f, vCol * 0.9f, vCol * 0.9f, vCol * 0.9f);
    }

    // --- 6. WEATHERED NATURAL STONE EXTERIOR VAULT (NO GREEN MOSS) ---
    // Smooth rounded weathered granite / shale mound enclosing the uneven tunnel along all rings
    const glm::vec3 extStoneMid(0.30f, 0.28f, 0.26f);
    const glm::vec3 extStoneDark(0.24f, 0.22f, 0.20f);
    const glm::vec3 extStoneRidge(0.35f, 0.32f, 0.28f);

    const int extVaultSegments = 12;
    float extDTheta = (float)M_PI / (float)extVaultSegments;

    for (int ringIdx = 0; ringIdx < numRingsZ; ++ringIdx)
    {
        float z0 = zStart - (float)ringIdx * ringStepZ;
        float z1 = z0 - ringStepZ;
        auto cs0 = getCrossSection(z0, zStart);
        auto cs1 = getCrossSection(z1, zStart);

        float extLeft0 = cs0.wLeft + 0.55f;
        float extRight0 = cs0.wRight + 0.55f;
        float extLeft1 = cs1.wLeft + 0.55f;
        float extRight1 = cs1.wRight + 0.55f;

        // Left outer stone wall
        addQuad(glm::vec3(cs1.xc - extLeft1, 0.0f, z1),
                glm::vec3(cs1.xc - extLeft1, cs1.springY, z1),
                glm::vec3(cs0.xc - extLeft0, cs0.springY, z0),
                glm::vec3(cs0.xc - extLeft0, 0.0f, z0),
                extStoneDark, extStoneMid, extStoneMid, extStoneDark);

        // Right outer stone wall
        addQuad(glm::vec3(cs0.xc + extRight0, 0.0f, z0),
                glm::vec3(cs0.xc + extRight0, cs0.springY, z0),
                glm::vec3(cs1.xc + extRight1, cs1.springY, z1),
                glm::vec3(cs1.xc + extRight1, 0.0f, z1),
                extStoneDark, extStoneMid, extStoneMid, extStoneDark);

        // Curved outer stone vault
        for (int v = 0; v < extVaultSegments; ++v)
        {
            float t0 = (float)M_PI - (float)v * extDTheta;
            float t1 = (float)M_PI - (float)(v + 1) * extDTheta;
            glm::vec3 col = (v >= 4 && v <= 7) ? extStoneRidge : extStoneMid;

            float tA = t0 / (float)M_PI;
            float tB = t1 / (float)M_PI;
            float r0A = glm::mix(extRight0, extLeft0, tA);
            float r0B = glm::mix(extRight0, extLeft0, tB);
            float r1A = glm::mix(extRight1, extLeft1, tA);
            float r1B = glm::mix(extRight1, extLeft1, tB);

            glm::vec3 ep0(cs0.xc + r0A * std::cos(t0), cs0.springY + r0A * std::sin(t0) + cs0.crownExtra * std::sin(t0), z0);
            glm::vec3 ep1(cs0.xc + r0B * std::cos(t1), cs0.springY + r0B * std::sin(t1) + cs0.crownExtra * std::sin(t1), z0);
            glm::vec3 ep2(cs1.xc + r1B * std::cos(t1), cs1.springY + r1B * std::sin(t1) + cs1.crownExtra * std::sin(t1), z1);
            glm::vec3 ep3(cs1.xc + r1A * std::cos(t0), cs1.springY + r1A * std::sin(t0) + cs1.crownExtra * std::sin(t0), z1);

            addQuad(ep0, ep1, ep2, ep3, col, col, col, col);
        }
    }

    // --- 7. SOLID RUSTIC COBBLESTONE DEAD-END BACK WALL (NO LIGHT AT THE END!) ---
    // User requested: "remove the light source on the end of tunnel and replace it with simple tunnel wall"
    auto csEnd = getCrossSection(zEnd, zStart);
    float sEnd = zStart - zEnd;
    glm::vec3 mColEnd = mortarBase * 0.90f;

    // A. Recessed mortar backing bulkhead at Z = zEnd - 0.015f
    // Lower rectangular mortar plane
    glm::vec3 mb0(csEnd.xc - csEnd.wLeft - 0.015f, 0.0f, zEnd - 0.015f);
    glm::vec3 mb1(csEnd.xc + csEnd.wRight + 0.015f, 0.0f, zEnd - 0.015f);
    glm::vec3 mb2(csEnd.xc + csEnd.wRight + 0.015f, csEnd.springY, zEnd - 0.015f);
    glm::vec3 mb3(csEnd.xc - csEnd.wLeft - 0.015f, csEnd.springY, zEnd - 0.015f);
    addQuad(mb0, mb1, mb2, mb3, mColEnd, mColEnd, mColEnd, mColEnd);

    // Upper arched mortar fan
    glm::vec3 mbCenter(csEnd.xc, csEnd.springY, zEnd - 0.015f);
    for (int a = 0; a < mortarArchSegments; ++a)
    {
        float t0 = (float)M_PI - (float)a * mDTheta;
        float t1 = (float)M_PI - (float)(a + 1) * mDTheta;

        glm::vec3 pA, nA, tgA;
        glm::vec3 pB, nB, tgB;
        getArchPoint(csEnd, t0, sEnd, pA, nA, tgA);
        getArchPoint(csEnd, t1, sEnd, pB, nB, tgB);
        pA.z = zEnd - 0.015f;
        pB.z = zEnd - 0.015f;

        addTriangle(mbCenter, pB, pA, mColEnd, mColEnd, mColEnd);
    }

    // B. Hand-laid rustic cobblestones on the lower back wall (4 courses x 7 columns)
    const int bwRows = 4;
    const int bwCols = 7;
    float bwRowH = (csEnd.springY - 0.08f) / (float)bwRows;
    float bwColW = (csEnd.wLeft + csEnd.wRight) / (float)bwCols;

    for (int r = 0; r < bwRows; ++r)
    {
        float baseY = 0.08f + ((float)r + 0.5f) * bwRowH;
        bool isStagger = (r % 2 == 1);

        for (int c = 0; c < bwCols; ++c)
        {
            float baseX = (csEnd.xc - csEnd.wLeft) + ((float)c + 0.5f) * bwColW;
            if (isStagger) baseX += bwColW * 0.45f;
            if (baseX < csEnd.xc - csEnd.wLeft + 0.12f || baseX > csEnd.xc + csEnd.wRight - 0.12f)
                continue;

            glm::vec3 P(baseX, baseY, zEnd);
            glm::vec3 N(0.0f, 0.0f, 1.0f);  // Inward normal (+Z, facing entrance)
            glm::vec3 T(1.0f, 0.0f, 0.0f);  // Horizontal tangent (+X)

            unsigned int seed = (unsigned int)(9917 + r * 137 + c * 83);
            int palIdx = seed % 8;
            float jitter = 0.92f + 0.16f * (float)(seed % 100) / 100.0f;
            glm::vec3 stoneCol = stonePalette[palIdx] * jitter;

            addCobblestone(P, N, T, bwColW * 0.88f, bwRowH * 0.88f, 0.088f, stoneCol, seed);
        }
    }

    // C. Hand-laid rustic cobblestones on the upper arch bulkhead (concentric curved courses)
    // Outer course near the arch rim (11 stones)
    const int bwArchOuterStones = 11;
    float bwaDTheta = (float)M_PI / (float)bwArchOuterStones;
    for (int i = 0; i < bwArchOuterStones; ++i)
    {
        float theta = (float)M_PI - ((float)i + 0.5f) * bwaDTheta;
        glm::vec3 pArch, nArch, tgArch;
        getArchPoint(csEnd, theta, sEnd, pArch, nArch, tgArch);

        glm::vec3 pCenter(csEnd.xc, csEnd.springY, 0.0f);
        glm::vec3 P = glm::mix(pCenter, pArch, 0.72f);
        P.z = zEnd;

        glm::vec3 N(0.0f, 0.0f, 1.0f);
        glm::vec3 radial = glm::normalize(pArch - pCenter);
        glm::vec3 T = glm::vec3(-radial.y, radial.x, 0.0f);

        unsigned int seed = (unsigned int)(13451 + i * 97);
        int palIdx = seed % 8;
        float jitter = 0.92f + 0.16f * (float)(seed % 100) / 100.0f;
        glm::vec3 stoneCol = stonePalette[palIdx] * jitter;

        float stoneW = 0.34f;
        float stoneH = 0.28f;
        addCobblestone(P, N, T, stoneW, stoneH, 0.088f, stoneCol, seed);
    }

    // Inner course filling the center of the arch bulkhead (5 stones)
    const int bwArchInnerStones = 5;
    float bwaInnerDTheta = (float)M_PI / (float)bwArchInnerStones;
    for (int i = 0; i < bwArchInnerStones; ++i)
    {
        float theta = (float)M_PI - ((float)i + 0.5f) * bwaInnerDTheta;
        glm::vec3 pArch, nArch, tgArch;
        getArchPoint(csEnd, theta, sEnd, pArch, nArch, tgArch);

        glm::vec3 pCenter(csEnd.xc, csEnd.springY, 0.0f);
        glm::vec3 P = glm::mix(pCenter, pArch, 0.36f);
        P.z = zEnd;

        glm::vec3 N(0.0f, 0.0f, 1.0f);
        glm::vec3 radial = glm::normalize(pArch - pCenter);
        glm::vec3 T = glm::vec3(-radial.y, radial.x, 0.0f);

        unsigned int seed = (unsigned int)(18923 + i * 113);
        int palIdx = seed % 8;
        float jitter = 0.92f + 0.16f * (float)(seed % 100) / 100.0f;
        glm::vec3 stoneCol = stonePalette[palIdx] * jitter;

        float stoneW = 0.32f;
        float stoneH = 0.26f;
        addCobblestone(P, N, T, stoneW, stoneH, 0.088f, stoneCol, seed);
    }

    // --- 8. REAR EXTERIOR STONE CAP ---
    // At Z = zEnd - 0.20f, completely sealing the back of the outer stone vault
    float zBack = zEnd - 0.20f;
    auto csBack = getCrossSection(zBack, zStart);
    float extLeftB = csBack.wLeft + 0.55f;
    float extRightB = csBack.wRight + 0.55f;

    addQuad(glm::vec3(csBack.xc - extLeftB, 0.0f, zBack),
            glm::vec3(csBack.xc - extLeftB, csBack.springY, zBack),
            glm::vec3(csBack.xc + extRightB, csBack.springY, zBack),
            glm::vec3(csBack.xc + extRightB, 0.0f, zBack),
            extStoneDark, extStoneDark, extStoneDark, extStoneDark);

    for (int v = 0; v < extVaultSegments; ++v)
    {
        float t0 = (float)M_PI - (float)v * extDTheta;
        float t1 = (float)M_PI - (float)(v + 1) * extDTheta;
        float tA = t0 / (float)M_PI;
        float tB = t1 / (float)M_PI;
        float rA = glm::mix(extRightB, extLeftB, tA);
        float rB = glm::mix(extRightB, extLeftB, tB);

        glm::vec3 cCenter(csBack.xc, csBack.springY, zBack);
        glm::vec3 c0(csBack.xc + rA * std::cos(t0), csBack.springY + rA * std::sin(t0), zBack);
        glm::vec3 c1(csBack.xc + rB * std::cos(t1), csBack.springY + rB * std::sin(t1), zBack);

        addTriangle(cCenter, c0, c1, extStoneDark, extStoneDark, extStoneDark);
    }

    // Upload vertices and indices to GPU buffer
    tunnelMesh.init(vertices, indices);
}

// ---------------------------------------------------------------------------
// Transformation Helper Methods
// ---------------------------------------------------------------------------
void Cave::drawCube(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                    const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                    float rotY, float rotX, float rotZ,
                    bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitCube.draw();
}

void Cave::drawPlane(const Shader& shader, const Plane& unitPlane, const glm::mat4& parentMat,
                     const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                     float rotY, float rotX, float rotZ,
                     bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitPlane.draw();
}

void Cave::drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                      const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                      bool emissive, const glm::vec3& emitColor) const
{
    drawSphere(shader, unitSphere, parentMat, offset, scale, color, 0.0f, 0.0f, 0.0f, emissive, emitColor);
}

void Cave::drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                      const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                      float rotY, float rotX, float rotZ,
                      bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitSphere.draw();
}

void Cave::drawCylinder(const Shader& shader, const Cylinder& unitCylinder, const glm::mat4& parentMat,
                        const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                        float rotY, float rotX, float rotZ,
                        bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitCylinder.draw();
}

void Cave::drawPyramid(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
                       const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                       float rotY, float rotX, float rotZ,
                       bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitPyramid.draw();
}



// ---------------------------------------------------------------------------
// 3D Controllable Flaming Torch Mesh
// ---------------------------------------------------------------------------
void Cave::drawTorchMesh(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                        const Sphere& unitSphere, const Pyramid& unitPyramid,
                        const glm::mat4& baseMat, float time) const
{
    // Materials
    const glm::vec3 woodShaft(0.26f, 0.16f, 0.10f);   // Aged rustic oak
    const glm::vec3 leatherWrap(0.38f, 0.25f, 0.14f); // Tanned leather binding
    const glm::vec3 ironBand(0.22f, 0.21f, 0.20f);    // Forged iron cage
    const glm::vec3 pitchBundle(0.11f, 0.09f, 0.08f); // Pitch & charred burlap bundle

    // 1. Wooden Handle / Shaft
    drawCylinder(shader, unitCylinder, baseMat,
                 glm::vec3(0.0f, 0.0f, 0.0f),
                 glm::vec3(0.045f, 0.55f, 0.045f),
                 woodShaft);

    // 2. Iron Pommel at base
    drawSphere(shader, unitSphere, baseMat,
               glm::vec3(0.0f, -0.01f, 0.0f),
               glm::vec3(0.052f, 0.04f, 0.052f),
               ironBand);

    // 3. Criss-Cross Leather Grip Wrapping on shaft
    drawCylinder(shader, unitCylinder, baseMat,
                 glm::vec3(0.0f, 0.14f, 0.0f),
                 glm::vec3(0.049f, 0.20f, 0.049f),
                 leatherWrap);

    // 4. Upper Iron Collar Ring
    drawCylinder(shader, unitCylinder, baseMat,
                 glm::vec3(0.0f, 0.53f, 0.0f),
                 glm::vec3(0.065f, 0.04f, 0.065f),
                 ironBand);

    // 5. Charred Burlap / Pitch Combustion Head
    drawCylinder(shader, unitCylinder, baseMat,
                 glm::vec3(0.0f, 0.56f, 0.0f),
                 glm::vec3(0.078f, 0.16f, 0.078f),
                 pitchBundle);

    // 6. Four Forged Iron Cage Ribs holding the burning bundle
    drawCube(shader, unitCube, baseMat, glm::vec3( 0.040f, 0.64f,  0.0f), glm::vec3(0.012f, 0.18f, 0.015f), ironBand);
    drawCube(shader, unitCube, baseMat, glm::vec3(-0.040f, 0.64f,  0.0f), glm::vec3(0.012f, 0.18f, 0.015f), ironBand);
    drawCube(shader, unitCube, baseMat, glm::vec3( 0.0f,   0.64f,  0.040f), glm::vec3(0.015f, 0.18f, 0.012f), ironBand);
    drawCube(shader, unitCube, baseMat, glm::vec3( 0.0f,   0.64f, -0.040f), glm::vec3(0.015f, 0.18f, 0.012f), ironBand);

    // 7. Dynamic Flaming Fire & Glowing Embers
    if (torchLit)
    {
        // Glowing ember bed at head of torch
        glm::vec3 coalGlow(1.0f, 0.42f, 0.06f);
        drawSphere(shader, unitSphere, baseMat,
                   glm::vec3(0.0f, 0.71f, 0.0f),
                   glm::vec3(0.072f, 0.038f, 0.072f),
                   coalGlow,
                   0.0f, 0.0f, 0.0f,
                   true, coalGlow * (1.3f * torchBrightness));

        // Animated Flame Harmonics
        float swayX = 0.014f * std::sin(time * 19.0f) + 0.009f * std::cos(time * 31.0f);
        float swayZ = 0.014f * std::cos(time * 17.0f) + 0.008f * std::sin(time * 29.0f);
        float flicker = (0.90f + 0.18f * std::sin(time * 23.0f) + 0.12f * std::cos(time * 37.0f)) * torchBrightness;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE); // Volumetric additive flame

        // A. Incandescent White-Hot Flame Core
        glm::vec3 coreCol(1.0f, 0.96f, 0.70f);
        drawPyramid(shader, unitPyramid, baseMat,
                    glm::vec3(swayX * 0.4f, 0.72f, swayZ * 0.4f),
                    glm::vec3(0.060f, 0.16f * flicker, 0.060f),
                    coreCol,
                    0.0f, 0.0f, 0.0f,
                    true, coreCol * 1.5f);

        // B. Middle Warm Golden Flame Tongue
        glm::vec3 midCol(1.0f, 0.65f, 0.15f);
        drawPyramid(shader, unitPyramid, baseMat,
                    glm::vec3(swayX * 0.8f, 0.72f, swayZ * 0.8f),
                    glm::vec3(0.090f, 0.25f * flicker, 0.090f),
                    midCol,
                    0.0f, 0.0f, 0.0f,
                    true, midCol * 1.3f);

        // C. Outer Licking Orange Flame Tip
        glm::vec3 outerCol(0.95f, 0.38f, 0.06f);
        drawPyramid(shader, unitPyramid, baseMat,
                    glm::vec3(swayX, 0.72f, swayZ),
                    glm::vec3(0.080f, 0.32f * flicker, 0.080f),
                    outerCol,
                    45.0f, 0.0f, 0.0f,
                    true, outerCol * 1.1f);

        // D. Drifting Glowing Fire Spark Embers
        for (int i = 0; i < 6; ++i)
        {
            float p = std::fmod(time * 1.5f + (float)i * 0.33f, 1.0f);
            float ey = 0.72f + p * 0.38f;
            float ex = swayX + 0.035f * std::sin(time * 6.0f + (float)i * 2.2f) * p;
            float ez = swayZ + 0.035f * std::cos(time * 7.0f + (float)i * 1.8f) * p;
            float es = 0.015f * (1.0f - p * 0.60f);

            glm::vec3 sparkCol(1.0f, 0.75f, 0.22f);
            drawSphere(shader, unitSphere, baseMat,
                       glm::vec3(ex, ey, ez),
                       glm::vec3(es),
                       sparkCol,
                       0.0f, 0.0f, 0.0f,
                       true, sparkCol * (1.2f * (1.0f - p)));
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    else
    {
        // Extinguished smoldering charcoal
        glm::vec3 deadCharcoal(0.20f, 0.06f, 0.02f);
        drawSphere(shader, unitSphere, baseMat,
                   glm::vec3(0.0f, 0.71f, 0.0f),
                   glm::vec3(0.045f, 0.022f, 0.045f),
                   deadCharcoal,
                   0.0f, 0.0f, 0.0f,
                   true, deadCharcoal * 0.25f);
    }
}

// ---------------------------------------------------------------------------
// First-Person Handheld Torch Rendering
// ---------------------------------------------------------------------------
void Cave::drawHandheldTorch(const Shader& shader,
                            const Cube& unitCube,
                            const Cylinder& unitCylinder,
                            const Sphere& unitSphere,
                            const Pyramid& unitPyramid,
                            const glm::vec3& camPos,
                            const glm::vec3& camFront,
                            const glm::vec3& camRight,
                            const glm::vec3& camUp,
                            float time) const
{
    // Torch is only drawn in first-person hand if player took it and it is lit
    if (!playerHasTorch || !torchLit) return;

    // Compact scale so the full torch (handle, grip, cage, coals, flame) is fully visible
    // "make the torch smaller to make full torch visible when i take it"
    const float torchScale = 0.35f;

    // Positioned smoothly in the lower-right foreground of the screen without clipping
    float bob = 0.005f * std::sin(time * 6.0f);
    glm::vec3 torchPos = camPos + camRight * 0.24f + camFront * 0.55f - camUp * (0.22f + bob);

    glm::mat4 tMat(1.0f);
    tMat = glm::translate(tMat, torchPos);

    // Orient along camera viewing direction
    glm::mat3 camBasis = glm::mat3(camRight, camUp, -camFront);
    tMat = tMat * glm::mat4(camBasis);

    // Tilt torch naturally towards center
    tMat = glm::rotate(tMat, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    tMat = glm::rotate(tMat, glm::radians(-12.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    tMat = glm::scale(tMat, glm::vec3(torchScale));

    drawTorchMesh(shader, unitCube, unitCylinder, unitSphere, unitPyramid, tMat, time);
}

// ---------------------------------------------------------------------------
// Torch Stand Outside the Cave Entrance
// "keep it outside the cave entrance, i can take it when i want."
// ---------------------------------------------------------------------------
void Cave::drawTorchStand(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                         const Sphere& unitSphere, const Pyramid& unitPyramid,
                         const glm::mat4& rootMat, float time) const
{
    glm::vec3 standBasePos(1.35f, 0.0f, 1.35f);
    glm::mat4 standMat = glm::translate(rootMat, standBasePos);

    const glm::vec3 stonePedestalCol(0.62f, 0.44f, 0.26f); // Warm sandstone
    const glm::vec3 ironCol(0.20f, 0.19f, 0.18f);          // Forged wrought iron

    // Dressed stone pedestal base sitting on the meadow grass outside
    drawCube(shader, unitCube, standMat, glm::vec3(0.0f, 0.10f, 0.0f), glm::vec3(0.28f, 0.20f, 0.28f), stonePedestalCol);

    // Iron vertical stand post
    drawCylinder(shader, unitCylinder, standMat, glm::vec3(0.0f, 0.20f, 0.0f), glm::vec3(0.045f, 0.72f, 0.045f), ironCol);

    // Forged iron sconce bracket & cup ring
    drawCylinder(shader, unitCylinder, standMat, glm::vec3(0.0f, 0.90f, 0.0f), glm::vec3(0.085f, 0.06f, 0.085f), ironCol);
    drawCylinder(shader, unitCylinder, standMat, glm::vec3(0.0f, 0.94f, 0.0f), glm::vec3(0.11f, 0.03f, 0.11f), ironCol);

    // If the player has NOT taken the torch, the burning flaming torch rests mounted outside on the stand!
    if (!playerHasTorch)
    {
        glm::mat4 torchMountMat = standMat;
        torchMountMat = glm::translate(torchMountMat, glm::vec3(0.0f, 0.58f, 0.0f));
        torchMountMat = glm::rotate(torchMountMat, glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        drawTorchMesh(shader, unitCube, unitCylinder, unitSphere, unitPyramid, torchMountMat, time);
    }
}

// ---------------------------------------------------------------------------
// Primary Render Pass
// ---------------------------------------------------------------------------
void Cave::draw(const Shader& shader,
                const Cube& unitCube,
                const TriangularPrism& unitPrism,
                const Pyramid& unitPyramid,
                const Sphere& unitSphere,
                const Plane& unitPlane,
                const Cylinder& unitCylinder,
                const Rock& unitRock,
                const CaveMotesState& motesState,
                float frameTime) const
{
    (void)unitPrism;
    (void)unitPlane;
    (void)unitRock;
    glm::mat4 rootMat = getRootMatrix();

    // 1. Render the procedural authentic cobblestone tunnel mesh
    shader.setMat4("model", rootMat);
    shader.setVec3("objectColor", glm::vec3(0.0f));
    shader.setBool("isEmissive", false);
    shader.setFloat("selfIllumination", 0.0f);
    tunnelMesh.draw();

    // 2. Render Torch Stand outside the entrance (holds the burning torch until player takes it)
    drawTorchStand(shader, unitCube, unitCylinder, unitSphere, unitPyramid, rootMat, frameTime);

    // 3. Render Floating Glowing Bioluminescent Motes throughout the cave tunnel
    drawFloatingMotes(shader, unitSphere, rootMat, motesState);

    // 4. Render the detailed open treasure chest and illuminating gold coins (modeled with cylinders)
    // Strictly localized golden illumination - only illuminates the immediate area!
    drawTreasureChestAndCoins(shader, unitCube, unitCylinder, unitSphere, unitPyramid, rootMat, frameTime);

    // Reset shader state
    shader.setBool("isEmissive", false);
    shader.setFloat("selfIllumination", 0.0f);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}

// Backward-compatibility overloads
void Cave::draw(const Shader& shader,
                const Cube& unitCube,
                const TriangularPrism& unitPrism,
                const Pyramid& unitPyramid,
                const Sphere& unitSphere,
                const Plane& unitPlane,
                const Rock& unitRock,
                const CaveMotesState& motesState) const
{
    static Cylinder defaultCylinder;
    draw(shader, unitCube, unitPrism, unitPyramid, unitSphere, unitPlane, defaultCylinder, unitRock, motesState, 0.0f);
}

void Cave::draw(const Shader& shader,
                const Cube& unitCube,
                const Pyramid& unitPyramid,
                const Sphere& unitSphere,
                const Plane& unitPlane,
                const Rock& unitRock,
                const CaveMotesState& motesState) const
{
    static TriangularPrism defaultPrism;
    static Cylinder defaultCylinder;
    draw(shader, unitCube, defaultPrism, unitPyramid, unitSphere, unitPlane, defaultCylinder, unitRock, motesState, 0.0f);
}

void Cave::draw(const Shader& shader,
                const Cube& unitCube,
                const TriangularPrism& unitPrism,
                const Pyramid& unitPyramid,
                const Sphere& unitSphere,
                const Plane& unitPlane,
                const Rock& unitRock) const
{
    CaveMotesState dummyState;
    static Cylinder defaultCylinder;
    draw(shader, unitCube, unitPrism, unitPyramid, unitSphere, unitPlane, defaultCylinder, unitRock, dummyState, 0.0f);
}

void Cave::draw(const Shader& shader,
                const Cube& unitCube,
                const Pyramid& unitPyramid,
                const Sphere& unitSphere,
                const Plane& unitPlane,
                const Rock& unitRock) const
{
    static TriangularPrism defaultPrism;
    CaveMotesState dummyState;
    static Cylinder defaultCylinder;
    draw(shader, unitCube, defaultPrism, unitPyramid, unitSphere, unitPlane, defaultCylinder, unitRock, dummyState, 0.0f);
}

// ---------------------------------------------------------------------------
// Soft Atmospheric Dim Light at the End of the Tunnel (DEPRECATED / REMOVED)
// Replaced by solid rustic cobblestone dead-end wall capping the tunnel.
// ---------------------------------------------------------------------------
void Cave::drawDimLightAtEnd(const Shader& shader, const Plane& unitPlane, const Cube& unitCube,
                            const Sphere& unitSphere, const glm::mat4& rootMat) const
{
    (void)shader;
    (void)unitPlane;
    (void)unitCube;
    (void)unitSphere;
    (void)rootMat;
}

// ---------------------------------------------------------------------------
// Floating Glowing Moving Elements Inside Cave (Bioluminescent Light Motes)
// "and where did the glowing moving elements inside the cave go?"
// ---------------------------------------------------------------------------
void Cave::drawFloatingMotes(const Shader& shader, const Sphere& unitSphere,
                             const glm::mat4& rootMat, const CaveMotesState& motesState) const
{
    if (motesState.motes.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive luminous blending
    glDepthMask(GL_FALSE);

    for (const auto& mote : motesState.motes)
    {
        // 1. Core glowing mote
        glm::mat4 m = glm::translate(rootMat, mote.localPos);
        m = glm::scale(m, glm::vec3(mote.scale * 2.2f));

        shader.setMat4("model", m);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setBool("isEmissive", true);
        shader.setVec3("emissionColor", mote.color * (mote.intensity * 2.4f));
        shader.setFloat("selfIllumination", 1.0f);
        unitSphere.draw();

        // 2. Soft outer ethereal glow aura
        glm::mat4 mAura = glm::translate(rootMat, mote.localPos);
        mAura = glm::scale(mAura, glm::vec3(mote.scale * 5.2f));
        shader.setMat4("model", mAura);
        shader.setVec3("objectColor", glm::vec3(1.0f));
        shader.setVec3("emissionColor", mote.color * (mote.intensity * 0.45f));
        unitSphere.draw();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    shader.setBool("isEmissive", false);
    shader.setFloat("selfIllumination", 0.0f);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}

// ---------------------------------------------------------------------------
// 3D Luminous Gold Coin Rendering using Cylinder Primitive
// "make eluminating gold coins with cylinders inside the cave"
// ---------------------------------------------------------------------------
void Cave::drawCoin(const Shader& shader,
                    const Cylinder& unitCylinder,
                    const glm::mat4& parentMat,
                    const glm::vec3& offset,
                    const glm::vec3& scale,
                    const glm::vec3& color,
                    float rotY, float rotX, float rotZ) const
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, offset);
    if (rotY != 0.0f) m = glm::rotate(m, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) m = glm::rotate(m, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) m = glm::rotate(m, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    // Center unit cylinder vertically so rotations tilt cleanly around disc center
    m = glm::translate(m, glm::vec3(0.0f, -scale.y * 0.5f, 0.0f));
    m = glm::scale(m, scale);

    shader.setMat4("model", parentMat * m);
    shader.setVec3("objectColor", color);
    shader.setFloat("shininess", 96.0f);
    shader.setFloat("selfIllumination", 0.0f);
    shader.setBool("isGoldCoin", true);
    unitCylinder.draw();
    shader.setBool("isGoldCoin", false);
}


// ---------------------------------------------------------------------------
// Detailed Open Treasure Chest & Illuminating Gold Coins
// (Strictly matches the user uploaded reference image: open arched chest,
//  signature golden lock plate, overflowing gold coins, cascading spill mound,
//  scattered coins trailing outward, sparkling gems, and hanging golden orb)
// "they shouldnt light up the whole cave, only the area around them"
// ---------------------------------------------------------------------------
void Cave::drawTreasureChestAndCoins(const Shader& shader,
                                    const Cube& unitCube,
                                    const Cylinder& unitCylinder,
                                    const Sphere& unitSphere,
                                    const Pyramid& unitPyramid,
                                    const glm::mat4& rootMat,
                                    float time) const
{
    // 1. Placement Coordinates on the Left Side of the Inner Cobblestone Sanctum
    const float zChest = -22.6f;
    TunnelCrossSection cs = getCrossSection(zChest, 0.55f);
    const float xChest = cs.xc - 0.72f;
    const float yChest = 0.0f;
    const float rotYChest = 24.0f; // Angled ~24 degrees inward towards tunnel center

    glm::mat4 chestMat = rootMat;
    chestMat = glm::translate(chestMat, glm::vec3(xChest, yChest, zChest));
    chestMat = glm::rotate(chestMat, glm::radians(rotYChest), glm::vec3(0.0f, 1.0f, 0.0f));

    // Upload Ray Tracing Bounds & State for the Treasure Hoard Environment
    shader.setBool("enableCoinRayTracing", m_coinRayTracingEnabled);
    shader.setVec3("treasureCenter", getTreasureWorldPos());
    shader.setVec3("hangingOrbPos", getHangingOrbWorldPos());
    shader.setVec3("chestMinBounds", getChestMinBounds());
    shader.setVec3("chestMaxBounds", getChestMaxBounds());

    // Color Palette

    const glm::vec3 woodDark(0.30f, 0.16f, 0.07f);       // Aged deep mahogany / oak
    const glm::vec3 woodMedium(0.36f, 0.20f, 0.09f);     // Warmer wood plank tone
    const glm::vec3 woodInner(0.18f, 0.09f, 0.04f);      // Interior tub shadow
    const glm::vec3 goldTrim(0.96f, 0.78f, 0.18f);       // Polished golden brass trim
    const glm::vec3 goldHighlight(1.00f, 0.90f, 0.32f);   // Bright gold studs & rivets
    const glm::vec3 darkMetal(0.12f, 0.10f, 0.08f);      // Dark keyhole slot

    // Luminous Gold Coin Colors
    const glm::vec3 goldBright(1.00f, 0.85f, 0.20f);
    const glm::vec3 goldWarm(0.96f, 0.77f, 0.14f);
    const glm::vec3 goldCanary(1.00f, 0.91f, 0.30f);
    const glm::vec3 goldAmber(0.92f, 0.70f, 0.15f);
    const glm::vec3 coinPal[4] = { goldBright, goldWarm, goldCanary, goldAmber };

    // Sparkling Gem Colors
    const glm::vec3 rubyRed(0.95f, 0.10f, 0.20f);
    const glm::vec3 sapphireBlue(0.15f, 0.40f, 0.98f);
    const glm::vec3 emeraldGreen(0.10f, 0.88f, 0.38f);
    const glm::vec3 amethystPurple(0.72f, 0.25f, 0.95f);

    // =======================================================================
    // 2. THE WOODEN CHEST BASE TUB
    // =======================================================================
    // Bottom base floor plate
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.025f, 0.0f), glm::vec3(1.08f, 0.05f, 0.68f), woodDark);
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.055f, 0.0f), glm::vec3(0.96f, 0.01f, 0.56f), woodInner);

    // Front wall (3 horizontal wooden planks)
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.11f, 0.315f), glm::vec3(0.96f, 0.11f, 0.05f), woodDark);
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.23f, 0.315f), glm::vec3(0.96f, 0.11f, 0.05f), woodMedium);
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.35f, 0.315f), glm::vec3(0.96f, 0.11f, 0.05f), woodDark);

    // Back wall
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.245f, -0.315f), glm::vec3(1.08f, 0.39f, 0.05f), woodDark);

    // Left and Right walls
    drawCube(shader, unitCube, chestMat, glm::vec3(-0.515f, 0.245f, 0.0f), glm::vec3(0.05f, 0.39f, 0.68f), woodDark);
    drawCube(shader, unitCube, chestMat, glm::vec3( 0.515f, 0.245f, 0.0f), glm::vec3(0.05f, 0.39f, 0.68f), woodMedium);

    // 4 Corner Golden L-Brackets with Studs
    for (int i = 0; i < 4; ++i)
    {
        float cx = (i % 2 == 0) ? -0.535f : 0.535f;
        float cz = (i / 2 == 0) ? -0.335f : 0.335f;
        drawCube(shader, unitCube, chestMat, glm::vec3(cx, 0.22f, cz), glm::vec3(0.065f, 0.44f, 0.018f), goldTrim);
        drawCube(shader, unitCube, chestMat, glm::vec3(cx, 0.22f, cz), glm::vec3(0.018f, 0.44f, 0.065f), goldTrim);

        drawSphere(shader, unitSphere, chestMat, glm::vec3(cx, 0.08f, cz), glm::vec3(0.024f), goldHighlight);
        drawSphere(shader, unitSphere, chestMat, glm::vec3(cx, 0.22f, cz), glm::vec3(0.024f), goldHighlight);
        drawSphere(shader, unitSphere, chestMat, glm::vec3(cx, 0.38f, cz), glm::vec3(0.024f), goldHighlight);
    }

    // Bottom Perimeter Gold Band Wrapping the Base
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.045f, 0.342f), glm::vec3(1.09f, 0.055f, 0.016f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.045f, -0.342f), glm::vec3(1.09f, 0.055f, 0.016f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3(-0.542f, 0.045f, 0.0f), glm::vec3(0.016f, 0.055f, 0.69f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3( 0.542f, 0.045f, 0.0f), glm::vec3(0.016f, 0.055f, 0.69f), goldTrim);

    // Top Perimeter Gold Lip Rim (Framing the Open Chest Mouth)
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.435f, 0.338f), glm::vec3(1.09f, 0.038f, 0.035f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3(0.0f, 0.435f, -0.338f), glm::vec3(1.09f, 0.038f, 0.035f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3(-0.538f, 0.435f, 0.0f), glm::vec3(0.035f, 0.038f, 0.69f), goldTrim);
    drawCube(shader, unitCube, chestMat, glm::vec3( 0.538f, 0.435f, 0.0f), glm::vec3(0.035f, 0.038f, 0.69f), goldTrim);

    // Front Vertical Reinforcing Gold Straps
    float strapFrontX[2] = { -0.24f, 0.24f };
    for (float sx : strapFrontX)
    {
        drawCube(shader, unitCube, chestMat, glm::vec3(sx, 0.23f, 0.342f), glm::vec3(0.048f, 0.36f, 0.016f), goldTrim);
        drawSphere(shader, unitSphere, chestMat, glm::vec3(sx, 0.12f, 0.352f), glm::vec3(0.022f), goldHighlight);
        drawSphere(shader, unitSphere, chestMat, glm::vec3(sx, 0.23f, 0.352f), glm::vec3(0.022f), goldHighlight);
        drawSphere(shader, unitSphere, chestMat, glm::vec3(sx, 0.34f, 0.352f), glm::vec3(0.022f), goldHighlight);
    }

    // Side Brass Carry Handles
    float handleX[2] = { -0.542f, 0.542f };
    for (float hx : handleX)
    {
        drawCube(shader, unitCube, chestMat, glm::vec3(hx, 0.25f, 0.0f), glm::vec3(0.015f, 0.07f, 0.11f), goldTrim);
        drawCylinder(shader, unitCylinder, chestMat, glm::vec3(hx, 0.22f, 0.0f), glm::vec3(0.025f, 0.09f, 0.09f), goldTrim, 0.0f, 0.0f, 90.0f);
    }

    // =======================================================================
    // 3. THE OPEN ARCHED / BARREL HINGED LID
    // =======================================================================
    glm::mat4 lidMat = chestMat;
    lidMat = glm::translate(lidMat, glm::vec3(0.0f, 0.44f, -0.335f));
    lidMat = glm::rotate(lidMat, glm::radians(-108.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Tilted backward open!

    const float R_arch = 0.34f;
    const int numSlats = 8;
    const float dPhi = 3.14159265f / (float)numSlats;
    for (int s = 0; s < numSlats; ++s)
    {
        float phi = ((float)s + 0.5f) * dPhi;
        float sy = R_arch * std::sin(phi);
        float sz = R_arch * (1.0f - std::cos(phi));
        float tiltDeg = glm::degrees(phi - 1.5707963f);
        glm::vec3 slatCol = (s % 2 == 0) ? woodDark : woodMedium;

        // Curved wooden slat plank
        drawCube(shader, unitCube, lidMat, glm::vec3(0.0f, sy, sz),
                 glm::vec3(1.06f, 0.038f, 0.145f), slatCol, 0.0f, tiltDeg, 0.0f);

        // 4 Arched Gold Metal Straps across the lid
        float strapX[4] = { -0.535f, -0.24f, 0.24f, 0.535f };
        for (int st = 0; st < 4; ++st)
        {
            drawCube(shader, unitCube, lidMat, glm::vec3(strapX[st], sy + 0.005f, sz),
                     glm::vec3(0.046f, 0.042f, 0.148f), goldTrim, 0.0f, tiltDeg, 0.0f);
            if (s % 2 == 1)
            {
                drawSphere(shader, unitSphere, lidMat, glm::vec3(strapX[st], sy + 0.024f, sz),
                           glm::vec3(0.022f), goldHighlight);
            }
        }
    }

    // Semicircular Wood End Caps on the Lid
    for (int e = 0; e < 5; ++e)
    {
        float ePhi = ((float)e + 0.5f) * (3.14159265f / 5.0f);
        float ey = R_arch * 0.52f * std::sin(ePhi);
        float ez = R_arch * (1.0f - 0.52f * std::cos(ePhi));
        drawCube(shader, unitCube, lidMat, glm::vec3(-0.532f, ey, ez), glm::vec3(0.035f, ey * 0.9f, 0.12f), woodDark);
        drawCube(shader, unitCube, lidMat, glm::vec3( 0.532f, ey, ez), glm::vec3(0.035f, ey * 0.9f, 0.12f), woodDark);
    }

    // Front opening lip rim of the lid
    drawCube(shader, unitCube, lidMat, glm::vec3(0.0f, 0.02f, 0.68f), glm::vec3(1.08f, 0.045f, 0.035f), goldTrim);

    // Signature Golden Shield Lock Plate with Keyhole (Prominently featured in reference image!)
    drawCube(shader, unitCube, lidMat, glm::vec3(0.0f, 0.035f, 0.705f), glm::vec3(0.18f, 0.045f, 0.14f), goldTrim);
    drawPyramid(shader, unitPyramid, lidMat, glm::vec3(0.0f, 0.02f, 0.775f), glm::vec3(0.16f, 0.09f, 0.04f), goldTrim, 0.0f, 90.0f, 0.0f);
    drawCube(shader, unitCube, lidMat, glm::vec3(0.0f, 0.058f, 0.705f), glm::vec3(0.028f, 0.02f, 0.055f), darkMetal);
    drawCylinder(shader, unitCylinder, lidMat, glm::vec3(0.0f, 0.058f, 0.685f), glm::vec3(0.045f, 0.02f, 0.045f), darkMetal);
    drawSphere(shader, unitSphere, lidMat, glm::vec3(-0.065f, 0.058f, 0.67f), glm::vec3(0.02f), goldHighlight);
    drawSphere(shader, unitSphere, lidMat, glm::vec3( 0.065f, 0.058f, 0.67f), glm::vec3(0.02f), goldHighlight);

    // =======================================================================
    // 4. ILLUMINATING GOLD COINS (MODELED WITH CYLINDERS)
    // =======================================================================
    // Group A: Base coin bed inside chest
    for (int ix = -2; ix <= 2; ++ix)
    {
        for (int iz = -1; iz <= 2; ++iz)
        {
            for (int layer = 0; layer < 2; ++layer)
            {
                unsigned int s = (unsigned int)(7129 + ix * 31 + iz * 17 + layer * 101);
                float jx = (float)ix * 0.18f + (stoneHash(s, 1) - 0.5f) * 0.08f;
                float jz = (float)iz * 0.13f - 0.04f + (stoneHash(s, 2) - 0.5f) * 0.08f;
                float jy = 0.18f + (float)layer * 0.08f + (stoneHash(s, 3) - 0.5f) * 0.03f;
                float tiltX = (stoneHash(s, 4) - 0.5f) * 26.0f;
                float tiltZ = (stoneHash(s, 5) - 0.5f) * 26.0f;
                float rotY = stoneHash(s, 6) * 360.0f;
                float rad = 0.155f + 0.025f * stoneHash(s, 7);
                glm::vec3 col = coinPal[s % 4];
                drawCoin(shader, unitCylinder, chestMat, glm::vec3(jx, jy, jz), glm::vec3(rad, 0.024f, rad), col, rotY, tiltX, tiltZ);
            }
        }
    }

    // Group B: Dense overflowing coin mound bulging up over the rim of the chest
    for (int i = 0; i < 44; ++i)
    {
        unsigned int s = (unsigned int)(8311 + i * 47);
        float u = (float)(i % 8) / 7.0f * 2.0f - 1.0f;
        float v = (float)(i / 8) / 5.0f * 2.0f - 1.0f;
        float cx = u * 0.40f + (stoneHash(s, 1) - 0.5f) * 0.06f;
        float cz = v * 0.22f + 0.06f + (stoneHash(s, 2) - 0.5f) * 0.06f;
        float dome = 1.0f - (cx * cx / 0.22f + (cz - 0.05f) * (cz - 0.05f) / 0.10f);
        dome = std::max(0.0f, dome);
        float cy = 0.34f + dome * 0.16f + (stoneHash(s, 3) - 0.5f) * 0.04f;
        float tiltX = (stoneHash(s, 4) - 0.5f) * 32.0f;
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 32.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.155f + 0.028f * stoneHash(s, 7);
        glm::vec3 col = coinPal[(s + 1) % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(cx, cy, cz), glm::vec3(rad, 0.025f, rad), col, rotY, tiltX, tiltZ);
    }

    // Group C: Coins cascading over the front and front-right rim onto the floor
    for (int i = 0; i < 22; ++i)
    {
        unsigned int s = (unsigned int)(9433 + i * 37);
        float t = (float)i / 21.0f;
        float sx = glm::mix(-0.15f, 0.42f, stoneHash(s, 1));
        float sz = 0.32f + t * 0.16f + (stoneHash(s, 2) - 0.5f) * 0.04f;
        float sy = glm::mix(0.44f, 0.10f, t) + (stoneHash(s, 3) - 0.5f) * 0.03f;
        float tiltX = 35.0f + 30.0f * stoneHash(s, 4); // Steep tilt down cascade
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 28.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.155f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[(s + 2) % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(sx, sy, sz), glm::vec3(rad, 0.024f, rad), col, rotY, tiltX, tiltZ);
    }

    // Group D: Large cascading ground mound in front and to the right of the chest
    // Tier 1: Upper mound on ground
    for (int i = 0; i < 18; ++i)
    {
        unsigned int s = (unsigned int)(10529 + i * 53);
        float mx = 0.05f + 0.45f * stoneHash(s, 1);
        float mz = 0.42f + 0.35f * stoneHash(s, 2);
        float my = 0.13f + 0.09f * stoneHash(s, 3);
        float tiltX = (stoneHash(s, 4) - 0.5f) * 24.0f;
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 24.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.16f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[s % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(mx, my, mz), glm::vec3(rad, 0.024f, rad), col, rotY, tiltX, tiltZ);
    }
    // Tier 2: Mid-slope mound
    for (int i = 0; i < 24; ++i)
    {
        unsigned int s = (unsigned int)(11677 + i * 41);
        float mx = -0.15f + 0.80f * stoneHash(s, 1);
        float mz = 0.48f + 0.55f * stoneHash(s, 2);
        float my = 0.06f + 0.08f * stoneHash(s, 3);
        float tiltX = (stoneHash(s, 4) - 0.5f) * 18.0f;
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 18.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.16f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[(s + 1) % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(mx, my, mz), glm::vec3(rad, 0.024f, rad), col, rotY, tiltX, tiltZ);
    }
    // Tier 3: Lower base apron on ground
    for (int i = 0; i < 26; ++i)
    {
        unsigned int s = (unsigned int)(12781 + i * 37);
        float mx = -0.28f + 1.15f * stoneHash(s, 1);
        float mz = 0.55f + 0.70f * stoneHash(s, 2);
        float my = 0.022f + 0.045f * stoneHash(s, 3);
        float tiltX = (stoneHash(s, 4) - 0.5f) * 12.0f;
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 12.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.155f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[(s + 2) % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(mx, my, mz), glm::vec3(rad, 0.022f, rad), col, rotY, tiltX, tiltZ);
    }

    // Group E: Scattered trailing coins leading out across the ground
    for (int i = 0; i < 36; ++i)
    {
        unsigned int s = (unsigned int)(13997 + i * 67);
        float tx, tz;
        if (i % 2 == 0)
        {
            tx = 0.75f + 1.10f * stoneHash(s, 1);
            tz = 0.15f + 1.25f * stoneHash(s, 2);
        }
        else
        {
            tx = -0.30f + 0.90f * stoneHash(s, 1);
            tz = 1.10f + 1.45f * stoneHash(s, 2);
        }
        float ty = 0.018f + 0.015f * stoneHash(s, 3);
        float tiltX = (stoneHash(s, 4) - 0.5f) * 8.0f;
        float tiltZ = (stoneHash(s, 5) - 0.5f) * 8.0f;
        float rotY = stoneHash(s, 6) * 360.0f;
        float rad = 0.155f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[(s + i) % 4];
        drawCoin(shader, unitCylinder, chestMat, glm::vec3(tx, ty, tz), glm::vec3(rad, 0.022f, rad), col, rotY, tiltX, tiltZ);
    }

    // =======================================================================
    // 5. SPARKLING CUT FACETED GEMS (RUBIES, SAPPHIRES, EMERALDS, AMETHYSTS)
    // =======================================================================
    auto drawFacetedGem = [&](const glm::vec3& gPos, const glm::vec3& gCol, float gScale, float gRotY)
    {
        glm::mat4 gm = chestMat;
        gm = glm::translate(gm, gPos);
        if (gRotY != 0.0f) gm = glm::rotate(gm, glm::radians(gRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        gm = glm::rotate(gm, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Upper crown facets
        shader.setMat4("model", gm * glm::scale(glm::mat4(1.0f), glm::vec3(gScale, gScale * 0.75f, gScale)));
        shader.setVec3("objectColor", gCol);
        shader.setFloat("shininess", 96.0f);
        shader.setFloat("selfIllumination", 0.40f);
        unitPyramid.draw();

        // Inverted lower pavilion facets
        glm::mat4 gmBot = glm::rotate(gm, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", gmBot * glm::scale(glm::mat4(1.0f), glm::vec3(gScale, gScale * 0.65f, gScale)));
        unitPyramid.draw();
    };

    drawFacetedGem(glm::vec3(-0.18f, 0.48f, 0.12f), rubyRed, 0.075f, 32.0f);
    drawFacetedGem(glm::vec3( 0.22f, 0.46f, 0.05f), sapphireBlue, 0.082f, 74.0f);
    drawFacetedGem(glm::vec3( 0.04f, 0.52f, 0.18f), emeraldGreen, 0.070f, 15.0f);
    drawFacetedGem(glm::vec3(-0.32f, 0.42f, -0.05f), amethystPurple, 0.088f, 55.0f);
    drawFacetedGem(glm::vec3( 0.28f, 0.22f, 0.42f), rubyRed, 0.078f, 120.0f);
    drawFacetedGem(glm::vec3( 0.48f, 0.12f, 0.65f), sapphireBlue, 0.085f, 40.0f);
    drawFacetedGem(glm::vec3( 0.12f, 0.11f, 0.72f), emeraldGreen, 0.072f, 88.0f);
    drawFacetedGem(glm::vec3( 0.65f, 0.06f, 0.88f), rubyRed, 0.068f, 14.0f);
    drawFacetedGem(glm::vec3(-0.08f, 0.07f, 0.82f), amethystPurple, 0.080f, 62.0f);

    // =======================================================================
    // 6. CEILING HANGING GOLDEN CHAINS & GLOWING ORB LANTERNS (Reference Art)
    // =======================================================================
    // Golden chain links hanging above the chest
    const int chainLinks = 6;
    for (int c = 0; c < chainLinks; ++c)
    {
        float cy = 2.45f - (float)c * 0.10f;
        drawCylinder(shader, unitCylinder, chestMat, glm::vec3(0.08f, cy - 0.05f, 0.12f),
                     glm::vec3(0.024f, 0.09f, 0.024f), goldTrim, (c % 2 == 0) ? 0.0f : 90.0f);
    }
    // Glowing golden sphere orb lantern
    glm::vec3 orbPos(0.08f, 1.82f, 0.12f);
    shader.setMat4("model", chestMat * glm::translate(glm::mat4(1.0f), orbPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.24f)));
    shader.setVec3("objectColor", glm::vec3(1.0f, 0.84f, 0.22f));
    shader.setFloat("shininess", 64.0f);
    shader.setFloat("selfIllumination", 0.82f);
    unitSphere.draw();
    // Brass meridian band around orb
    drawCylinder(shader, unitCylinder, chestMat, orbPos - glm::vec3(0.0f, 0.015f, 0.0f),
                 glm::vec3(0.255f, 0.03f, 0.255f), goldTrim);

    // Secondary hanging chain with faceted amethyst crystal
    for (int c = 0; c < 5; ++c)
    {
        float cy = 2.42f - (float)c * 0.09f;
        drawCylinder(shader, unitCylinder, chestMat, glm::vec3(0.72f, cy - 0.045f, -0.22f),
                     glm::vec3(0.020f, 0.08f, 0.020f), goldTrim, (c % 2 == 0) ? 0.0f : 90.0f);
    }
    drawFacetedGem(glm::vec3(0.72f, 1.95f, -0.22f), amethystPurple, 0.14f, 45.0f);

    // =======================================================================
    // 7. SECONDARY TREASURE CHESTS (Flanking the gold hoard matching reference art)
    // =======================================================================
    // Right arched chest resting next to the cascading gold mound
    drawClosedChest(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                    rootMat, glm::vec3(cs.xc + 0.68f, 0.0f, -22.15f), -26.0f, 0.72f);

    // Left rear smaller chest tucked into the corner alcove
    drawClosedChest(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                    rootMat, glm::vec3(cs.xc - 1.08f, 0.0f, -23.45f), 38.0f, 0.58f);

    // Scattered gold coins near the braziers and floor chests
    for (int i = 0; i < 12; ++i)
    {
        unsigned int s = (unsigned int)(14891 + i * 29);
        float bx = (i % 2 == 0) ? (cs.xc - 0.05f + (stoneHash(s, 1) - 0.5f) * 0.40f)
                                : (cs.xc + 1.05f + (stoneHash(s, 1) - 0.5f) * 0.45f);
        float bz = (i % 2 == 0) ? (-23.35f + (stoneHash(s, 2) - 0.5f) * 0.40f)
                                : (-22.35f + (stoneHash(s, 2) - 0.5f) * 0.45f);
        float by = 0.015f + 0.010f * stoneHash(s, 3);
        float rad = 0.155f + 0.025f * stoneHash(s, 7);
        glm::vec3 col = coinPal[s % 4];
        drawCoin(shader, unitCylinder, rootMat, glm::vec3(bx, by, bz), glm::vec3(rad, 0.022f, rad), col, stoneHash(s, 6) * 360.0f);
    }

    // =======================================================================
    // 8. MOUNTED WALL SCONCES & STANDING BRAZIERS (Blazing Fire Around Treasure)
    // =======================================================================
    TunnelCrossSection csFront = getCrossSection(-21.7f, 0.55f);
    TunnelCrossSection csBack = getCrossSection(-23.6f, 0.55f);

    // Left Wall Mounted Torches
    drawWallTorchSconce(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(csFront.xc - csFront.wLeft + 0.08f, 1.45f, -21.7f),
                        -1.0f, time, 0.0f);
    drawWallTorchSconce(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(csBack.xc - csBack.wLeft + 0.08f, 1.48f, -23.6f),
                        -1.0f, time, 1.7f);

    // Right Wall Mounted Torches
    drawWallTorchSconce(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(csFront.xc + csFront.wRight - 0.08f, 1.45f, -21.7f),
                        1.0f, time, 3.2f);
    drawWallTorchSconce(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(csBack.xc + csBack.wRight - 0.08f, 1.48f, -23.6f),
                        1.0f, time, 4.9f);

    // Central Standing Brazier Staff (Rising directly behind central gold mound)
    drawStandingBrazier(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(cs.xc - 0.05f, 0.0f, -23.35f),
                        time, 2.4f);

    // Secondary Standing Brazier Staff (Right flank behind secondary chest)
    drawStandingBrazier(shader, unitCube, unitCylinder, unitSphere, unitPyramid,
                        rootMat, glm::vec3(cs.xc + 1.15f, 0.0f, -22.35f),
                        time, 5.1f);

    // =======================================================================
    // 9. RESTORE SHADER STATE
    // =======================================================================
    shader.setFloat("selfIllumination", 0.0f);
    shader.setFloat("shininess", 32.0f);
    shader.setBool("isEmissive", false);
    shader.setBool("isGoldCoin", false);
    shader.setBool("enableCoinRayTracing", false);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}

// ---------------------------------------------------------------------------
// Atmospheric Wall Torch Sconce with Multi-Tier Animated Flame & Rising Sparks
// ---------------------------------------------------------------------------
void Cave::drawWallTorchSconce(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                              const Sphere& unitSphere, const Pyramid& unitPyramid,
                              const glm::mat4& rootMat, const glm::vec3& pos, float wallSide,
                              float time, float phase) const
{
    glm::mat4 sconceMat = rootMat;
    sconceMat = glm::translate(sconceMat, pos);

    const glm::vec3 ironCol(0.18f, 0.17f, 0.16f);
    const glm::vec3 pitchBundle(0.16f, 0.12f, 0.09f);
    const glm::vec3 coalGlow(1.0f, 0.45f, 0.08f);

    float inwardDir = (wallSide < 0.0f) ? 1.0f : -1.0f;

    // 1. Heavy forged iron wall mounting flange anchored to stone cavern wall
    drawCube(shader, unitCube, sconceMat, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.035f, 0.22f, 0.14f), ironCol);
    drawSphere(shader, unitSphere, sconceMat, glm::vec3(0.015f * inwardDir, 0.08f, 0.0f), glm::vec3(0.018f), ironCol * 1.3f);
    drawSphere(shader, unitSphere, sconceMat, glm::vec3(0.015f * inwardDir, -0.08f, 0.0f), glm::vec3(0.018f), ironCol * 1.3f);

    // 2. Angled forged iron arm extending out into the cavern
    float armDist = 0.28f;
    glm::vec3 headPos(armDist * inwardDir, 0.08f, 0.0f);
    drawCylinder(shader, unitCylinder, sconceMat,
                 glm::vec3(armDist * 0.5f * inwardDir, 0.02f, 0.0f),
                 glm::vec3(0.024f, armDist, 0.024f),
                 ironCol, 0.0f, 0.0f, 90.0f * inwardDir);

    // Diagonal supporting strut
    drawCylinder(shader, unitCylinder, sconceMat,
                 glm::vec3(armDist * 0.45f * inwardDir, -0.06f, 0.0f),
                 glm::vec3(0.018f, armDist * 0.85f, 0.018f),
                 ironCol, 0.0f, 0.0f, 45.0f * inwardDir);

    // 3. Forged iron cup holder ring & collar basket
    drawCylinder(shader, unitCylinder, sconceMat, headPos,
                 glm::vec3(0.095f, 0.045f, 0.095f), ironCol);
    drawCylinder(shader, unitCylinder, sconceMat, headPos + glm::vec3(0.0f, 0.06f, 0.0f),
                 glm::vec3(0.115f, 0.025f, 0.115f), ironCol);

    // 4. Torch shaft / charred burlap pitch combustion bundle
    drawCylinder(shader, unitCylinder, sconceMat, headPos + glm::vec3(0.0f, -0.10f, 0.0f),
                 glm::vec3(0.052f, 0.32f, 0.052f), pitchBundle);

    // Iron cage ribs holding the head
    for (int r = 0; r < 4; ++r)
    {
        float ang = glm::radians((float)r * 90.0f);
        float rx = std::cos(ang) * 0.055f;
        float rz = std::sin(ang) * 0.055f;
        drawCube(shader, unitCube, sconceMat, headPos + glm::vec3(rx, 0.04f, rz),
                 glm::vec3(0.012f, 0.14f, 0.012f), ironCol);
    }

    // 5. Glowing ember bed at torch head
    glm::vec3 flameRoot = headPos + glm::vec3(0.0f, 0.11f, 0.0f);
    drawSphere(shader, unitSphere, sconceMat, flameRoot,
               glm::vec3(0.082f, 0.042f, 0.082f), coalGlow,
               true, coalGlow * 1.6f);

    // 6. Dynamic multi-tier animated flaming fire
    float t = time + phase;
    float swayX = 0.016f * std::sin(t * 18.0f) + 0.009f * std::cos(t * 31.0f);
    float swayZ = 0.016f * std::cos(t * 16.0f) + 0.008f * std::sin(t * 27.0f);
    float flicker = 0.92f + 0.22f * std::sin(t * 22.0f) + 0.14f * std::cos(t * 36.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE); // Volumetric additive flame

    // A. Incandescent white-yellow flame core
    glm::vec3 coreCol(1.0f, 0.96f, 0.72f);
    drawPyramid(shader, unitPyramid, sconceMat,
                flameRoot + glm::vec3(swayX * 0.35f, 0.01f, swayZ * 0.35f),
                glm::vec3(0.085f, 0.20f * flicker, 0.085f),
                coreCol, 0.0f, 0.0f, 0.0f, true, coreCol * 1.6f);

    // B. Warm golden flame body
    glm::vec3 midCol(1.0f, 0.64f, 0.14f);
    drawPyramid(shader, unitPyramid, sconceMat,
                flameRoot + glm::vec3(swayX * 0.75f, 0.01f, swayZ * 0.75f),
                glm::vec3(0.125f, 0.32f * flicker, 0.125f),
                midCol, 0.0f, 0.0f, 0.0f, true, midCol * 1.4f);

    // C. Outer licking orange flame tip
    glm::vec3 outerCol(0.96f, 0.36f, 0.06f);
    drawPyramid(shader, unitPyramid, sconceMat,
                flameRoot + glm::vec3(swayX, 0.01f, swayZ),
                glm::vec3(0.110f, 0.42f * flicker, 0.110f),
                outerCol, 45.0f, 0.0f, 0.0f, true, outerCol * 1.2f);

    // D. Drifting glowing embers rising toward cavern vault
    for (int i = 0; i < 6; ++i)
    {
        float p = std::fmod(t * 1.4f + (float)i * 0.34f, 1.0f);
        float ey = flameRoot.y + 0.04f + p * 0.45f;
        float ex = flameRoot.x + swayX + 0.04f * std::sin(t * 5.5f + (float)i * 2.1f) * p;
        float ez = flameRoot.z + swayZ + 0.04f * std::cos(t * 6.5f + (float)i * 1.7f) * p;
        float es = 0.018f * (1.0f - p * 0.65f);

        glm::vec3 sparkCol(1.0f, 0.76f, 0.24f);
        drawSphere(shader, unitSphere, sconceMat,
                   glm::vec3(ex, ey, ez),
                   glm::vec3(es),
                   sparkCol,
                   true, sparkCol * (1.5f * (1.0f - p)));
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// Standing Torch Brazier Staff Rising Behind the Central Treasure Hoard
// ---------------------------------------------------------------------------
void Cave::drawStandingBrazier(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                              const Sphere& unitSphere, const Pyramid& unitPyramid,
                              const glm::mat4& rootMat, const glm::vec3& pos,
                              float time, float phase) const
{
    glm::mat4 bMat = rootMat;
    bMat = glm::translate(bMat, pos);

    const glm::vec3 ironCol(0.19f, 0.18f, 0.17f);
    const glm::vec3 bronzeRing(0.68f, 0.52f, 0.22f);
    const glm::vec3 pitchBundle(0.15f, 0.11f, 0.08f);
    const glm::vec3 coalGlow(1.0f, 0.44f, 0.07f);

    // 1. Triangular tripod / heavy forged circular base plate on cavern floor
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 0.03f, 0.0f), glm::vec3(0.24f, 0.05f, 0.24f), ironCol);
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 0.07f, 0.0f), glm::vec3(0.16f, 0.04f, 0.16f), ironCol);

    // Splayed tripod support struts
    for (int leg = 0; leg < 3; ++leg)
    {
        float lang = glm::radians((float)leg * 120.0f);
        float lx = std::cos(lang) * 0.14f;
        float lz = std::sin(lang) * 0.14f;
        drawCube(shader, unitCube, bMat, glm::vec3(lx, 0.08f, lz), glm::vec3(0.035f, 0.14f, 0.035f), ironCol, glm::degrees(lang), 18.0f, 0.0f);
    }

    // 2. Slender vertical iron staff / post rising up (height ~1.25m)
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 0.10f, 0.0f), glm::vec3(0.045f, 1.05f, 0.045f), ironCol);

    // Decorative bronze forged rings along shaft
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 0.45f, 0.0f), glm::vec3(0.065f, 0.035f, 0.065f), bronzeRing);
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 0.85f, 0.0f), glm::vec3(0.065f, 0.035f, 0.065f), bronzeRing);

    // 3. Flared upper brazier cup & iron basket
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 1.15f, 0.0f), glm::vec3(0.11f, 0.06f, 0.11f), ironCol);
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 1.20f, 0.0f), glm::vec3(0.16f, 0.04f, 0.16f), ironCol);
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 1.23f, 0.0f), glm::vec3(0.18f, 0.025f, 0.18f), bronzeRing);

    // Pitch bundle inside the cup
    drawCylinder(shader, unitCylinder, bMat, glm::vec3(0.0f, 1.22f, 0.0f), glm::vec3(0.14f, 0.10f, 0.14f), pitchBundle);

    // Glowing ember bed
    glm::vec3 flameRoot(0.0f, 1.30f, 0.0f);
    drawSphere(shader, unitSphere, bMat, flameRoot, glm::vec3(0.115f, 0.055f, 0.115f), coalGlow,
               true, coalGlow * 1.6f);

    // 4. Dynamic animated flame
    float t = time + phase;
    float swayX = 0.018f * std::sin(t * 17.0f) + 0.010f * std::cos(t * 33.0f);
    float swayZ = 0.018f * std::cos(t * 19.0f) + 0.009f * std::sin(t * 29.0f);
    float flicker = 0.90f + 0.22f * std::sin(t * 23.0f) + 0.14f * std::cos(t * 37.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    // A. Core incandescent pyramid
    glm::vec3 coreCol(1.0f, 0.97f, 0.74f);
    drawPyramid(shader, unitPyramid, bMat,
                flameRoot + glm::vec3(swayX * 0.35f, 0.01f, swayZ * 0.35f),
                glm::vec3(0.10f, 0.24f * flicker, 0.10f),
                coreCol, 0.0f, 0.0f, 0.0f, true, coreCol * 1.6f);

    // B. Warm golden flame body
    glm::vec3 midCol(1.0f, 0.65f, 0.14f);
    drawPyramid(shader, unitPyramid, bMat,
                flameRoot + glm::vec3(swayX * 0.75f, 0.01f, swayZ * 0.75f),
                glm::vec3(0.15f, 0.38f * flicker, 0.15f),
                midCol, 0.0f, 0.0f, 0.0f, true, midCol * 1.4f);

    // C. Outer flickering flame tip
    glm::vec3 outerCol(0.96f, 0.35f, 0.05f);
    drawPyramid(shader, unitPyramid, bMat,
                flameRoot + glm::vec3(swayX, 0.01f, swayZ),
                glm::vec3(0.13f, 0.50f * flicker, 0.13f),
                outerCol, 45.0f, 0.0f, 0.0f, true, outerCol * 1.2f);

    // D. Drifting sparks / glowing embers rising into the ceiling vault
    for (int i = 0; i < 7; ++i)
    {
        float p = std::fmod(t * 1.35f + (float)i * 0.31f, 1.0f);
        float ey = flameRoot.y + 0.05f + p * 0.55f;
        float ex = flameRoot.x + swayX + 0.045f * std::sin(t * 5.0f + (float)i * 1.9f) * p;
        float ez = flameRoot.z + swayZ + 0.045f * std::cos(t * 6.0f + (float)i * 2.3f) * p;
        float es = 0.020f * (1.0f - p * 0.65f);

        glm::vec3 sparkCol(1.0f, 0.78f, 0.25f);
        drawSphere(shader, unitSphere, bMat,
                   glm::vec3(ex, ey, ez),
                   glm::vec3(es),
                   sparkCol,
                   true, sparkCol * (1.5f * (1.0f - p)));
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// Secondary Closed Arched Treasure Chest Flanking the Hoard
// ---------------------------------------------------------------------------
void Cave::drawClosedChest(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                          const Sphere& unitSphere, const Pyramid& unitPyramid,
                          const glm::mat4& rootMat, const glm::vec3& pos, float rotY, float scale) const
{
    glm::mat4 cMat = rootMat;
    cMat = glm::translate(cMat, pos);
    if (rotY != 0.0f) cMat = glm::rotate(cMat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    cMat = glm::scale(cMat, glm::vec3(scale));

    const glm::vec3 woodDark(0.28f, 0.15f, 0.07f);
    const glm::vec3 woodMedium(0.35f, 0.19f, 0.09f);
    const glm::vec3 goldTrim(0.96f, 0.78f, 0.18f);
    const glm::vec3 goldHighlight(1.00f, 0.90f, 0.32f);
    const glm::vec3 darkMetal(0.12f, 0.10f, 0.08f);

    // 1. Chest Main Tub Box
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.22f, 0.0f), glm::vec3(0.85f, 0.44f, 0.54f), woodDark);
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.22f, 0.272f), glm::vec3(0.82f, 0.14f, 0.015f), woodMedium);

    // 4 Corner Golden Brackets & Studs
    for (int i = 0; i < 4; ++i)
    {
        float cx = (i % 2 == 0) ? -0.425f : 0.425f;
        float cz = (i / 2 == 0) ? -0.27f : 0.27f;
        drawCube(shader, unitCube, cMat, glm::vec3(cx, 0.22f, cz), glm::vec3(0.055f, 0.44f, 0.016f), goldTrim);
        drawCube(shader, unitCube, cMat, glm::vec3(cx, 0.22f, cz), glm::vec3(0.016f, 0.44f, 0.055f), goldTrim);
        drawSphere(shader, unitSphere, cMat, glm::vec3(cx, 0.10f, cz), glm::vec3(0.020f), goldHighlight);
        drawSphere(shader, unitSphere, cMat, glm::vec3(cx, 0.35f, cz), glm::vec3(0.020f), goldHighlight);
    }

    // Bottom Gold Band
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.035f, 0.0f), glm::vec3(0.86f, 0.05f, 0.55f), goldTrim);

    // Lip Rim Band
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.44f, 0.0f), glm::vec3(0.87f, 0.04f, 0.56f), goldTrim);

    // 2. Closed Arched Barrel Lid
    const float R_arch = 0.27f;
    const int numSlats = 7;
    const float dPhi = 3.14159265f / (float)numSlats;
    for (int s = 0; s < numSlats; ++s)
    {
        float phi = ((float)s + 0.5f) * dPhi;
        float sy = 0.44f + R_arch * std::sin(phi);
        float sz = -R_arch * std::cos(phi);
        float tiltDeg = glm::degrees(phi - 1.5707963f);
        glm::vec3 slatCol = (s % 2 == 0) ? woodDark : woodMedium;

        drawCube(shader, unitCube, cMat, glm::vec3(0.0f, sy, sz),
                 glm::vec3(0.84f, 0.035f, 0.13f), slatCol, 0.0f, tiltDeg, 0.0f);

        // Gold straps over closed lid
        float strapX[3] = { -0.42f, 0.0f, 0.42f };
        for (float sx : strapX)
        {
            drawCube(shader, unitCube, cMat, glm::vec3(sx, sy + 0.004f, sz),
                     glm::vec3(0.045f, 0.038f, 0.132f), goldTrim, 0.0f, tiltDeg, 0.0f);
            if (s % 2 == 1)
            {
                drawSphere(shader, unitSphere, cMat, glm::vec3(sx, sy + 0.022f, sz), glm::vec3(0.020f), goldHighlight);
            }
        }
    }

    // Golden front lock latch
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.42f, 0.285f), glm::vec3(0.14f, 0.12f, 0.025f), goldTrim);
    drawPyramid(shader, unitPyramid, cMat, glm::vec3(0.0f, 0.35f, 0.285f), glm::vec3(0.12f, 0.07f, 0.025f), goldTrim, 0.0f, 180.0f, 0.0f);
    drawCube(shader, unitCube, cMat, glm::vec3(0.0f, 0.41f, 0.30f), glm::vec3(0.024f, 0.045f, 0.01f), darkMetal);

    // Side carry handles
    drawCylinder(shader, unitCylinder, cMat, glm::vec3(-0.435f, 0.24f, 0.0f), glm::vec3(0.022f, 0.08f, 0.08f), goldTrim, 0.0f, 0.0f, 90.0f);
    drawCylinder(shader, unitCylinder, cMat, glm::vec3( 0.435f, 0.24f, 0.0f), glm::vec3(0.022f, 0.08f, 0.08f), goldTrim, 0.0f, 0.0f, 90.0f);
}


