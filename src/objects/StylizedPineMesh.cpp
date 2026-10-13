#include "objects/StylizedPineMesh.h"
#include "texture/TextureManager.h"
#include <cmath>
#include <algorithm>

namespace {

#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

struct TreeParams {
    float totalHeight;
    float trunkBaseRadius;
    float canopyStartHeight;
    int numTiers;
    float maxCanopyRadius;
    float canopyTaperPower;
    float leanX;
    float leanZ;
    float boughDroopFactor;
    int deadTwigCount;
    float deadTwigLength;
};

TreeParams getParams(int variant)
{
    TreeParams p;
    switch (variant % 5)
    {
    case 0: // Tree 1: Compact Dwarf Pine (short, stocky, wide base, 4 dense tiers)
        p.totalHeight        = 4.8f;
        p.trunkBaseRadius    = 0.30f;
        p.canopyStartHeight  = 0.95f;
        p.numTiers           = 4;
        p.maxCanopyRadius    = 2.15f;
        p.canopyTaperPower   = 0.88f;
        p.leanX              = 0.0f;
        p.leanZ              = 0.0f;
        p.boughDroopFactor   = 0.34f;
        p.deadTwigCount      = 5;
        p.deadTwigLength     = 0.65f;
        break;

    case 1: // Tree 2: Slender Mountain Pine (tall, narrow spire, 7 tiers, gentle tilt)
        p.totalHeight        = 7.6f;
        p.trunkBaseRadius    = 0.26f;
        p.canopyStartHeight  = 1.35f;
        p.numTiers           = 7;
        p.maxCanopyRadius    = 2.05f;
        p.canopyTaperPower   = 0.95f;
        p.leanX              = 0.18f;
        p.leanZ              = -0.10f;
        p.boughDroopFactor   = 0.38f;
        p.deadTwigCount      = 6;
        p.deadTwigLength     = 0.70f;
        break;

    case 2: // Tree 3: Grand Hero Pine (centerpiece majestic pine, straight trunk, 9 tiers, prominent dead twigs)
        p.totalHeight        = 9.4f;
        p.trunkBaseRadius    = 0.36f;
        p.canopyStartHeight  = 1.60f;
        p.numTiers           = 9;
        p.maxCanopyRadius    = 2.65f;
        p.canopyTaperPower   = 0.92f;
        p.leanX              = 0.0f;
        p.leanZ              = 0.0f;
        p.boughDroopFactor   = 0.42f;
        p.deadTwigCount      = 8;
        p.deadTwigLength     = 0.85f;
        break;

    case 3: // Tree 4: Wind-Swept Bent Pine (curved arched trunk leaning gracefully in the wind)
        p.totalHeight        = 8.2f;
        p.trunkBaseRadius    = 0.28f;
        p.canopyStartHeight  = 1.40f;
        p.numTiers           = 7;
        p.maxCanopyRadius    = 2.20f;
        p.canopyTaperPower   = 0.95f;
        p.leanX              = 1.25f;
        p.leanZ              = 0.25f;
        p.boughDroopFactor   = 0.40f;
        p.deadTwigCount      = 6;
        p.deadTwigLength     = 0.72f;
        break;

    case 4: // Tree 5: Alpine Stunted Pine (broad lower skirt, asymmetric crown, tilted spire)
    default:
        p.totalHeight        = 5.9f;
        p.trunkBaseRadius    = 0.32f;
        p.canopyStartHeight  = 1.15f;
        p.numTiers           = 5;
        p.maxCanopyRadius    = 2.30f;
        p.canopyTaperPower   = 0.85f;
        p.leanX              = -0.35f;
        p.leanZ              = 0.15f;
        p.boughDroopFactor   = 0.44f;
        p.deadTwigCount      = 6;
        p.deadTwigLength     = 0.75f;
        break;
    }
    return p;
}

struct FoliageColors {
    glm::vec3 tipCenter;
    glm::vec3 tipSide;
    glm::vec3 valley;
    glm::vec3 midGreen;
    glm::vec3 ridgeMid;
    glm::vec3 deepTeal;
    glm::vec3 underside;
    glm::vec3 spireTip;
    glm::vec3 spireMid;
    glm::vec3 spireBase;
};

FoliageColors getFoliageColors(int paletteType)
{
    FoliageColors c;
    switch (paletteType % 5)
    {
    case 1: // Autumn Amber Orange
        c.tipCenter  = glm::vec3(0.98f, 0.66f, 0.12f);
        c.tipSide    = glm::vec3(0.92f, 0.54f, 0.10f);
        c.valley     = glm::vec3(0.68f, 0.32f, 0.06f);
        c.midGreen   = glm::vec3(0.86f, 0.42f, 0.08f);
        c.ridgeMid   = glm::vec3(0.96f, 0.56f, 0.14f);
        c.deepTeal   = glm::vec3(0.42f, 0.16f, 0.05f);
        c.underside  = glm::vec3(0.35f, 0.12f, 0.04f);
        c.spireTip   = glm::vec3(1.00f, 0.74f, 0.16f);
        c.spireMid   = glm::vec3(0.90f, 0.48f, 0.10f);
        c.spireBase  = glm::vec3(0.72f, 0.32f, 0.06f);
        break;

    case 2: // Autumn Golden Saffron
        c.tipCenter  = glm::vec3(1.00f, 0.88f, 0.22f);
        c.tipSide    = glm::vec3(0.96f, 0.78f, 0.18f);
        c.valley     = glm::vec3(0.76f, 0.50f, 0.08f);
        c.midGreen   = glm::vec3(0.90f, 0.68f, 0.12f);
        c.ridgeMid   = glm::vec3(0.98f, 0.82f, 0.20f);
        c.deepTeal   = glm::vec3(0.50f, 0.30f, 0.06f);
        c.underside  = glm::vec3(0.42f, 0.24f, 0.05f);
        c.spireTip   = glm::vec3(1.00f, 0.94f, 0.28f);
        c.spireMid   = glm::vec3(0.94f, 0.76f, 0.16f);
        c.spireBase  = glm::vec3(0.78f, 0.52f, 0.09f);
        break;

    case 3: // Autumn Crimson Scarlet
        c.tipCenter  = glm::vec3(0.94f, 0.34f, 0.14f);
        c.tipSide    = glm::vec3(0.88f, 0.26f, 0.12f);
        c.valley     = glm::vec3(0.58f, 0.13f, 0.08f);
        c.midGreen   = glm::vec3(0.76f, 0.18f, 0.10f);
        c.ridgeMid   = glm::vec3(0.90f, 0.30f, 0.14f);
        c.deepTeal   = glm::vec3(0.40f, 0.09f, 0.05f);
        c.underside  = glm::vec3(0.32f, 0.07f, 0.04f);
        c.spireTip   = glm::vec3(0.98f, 0.44f, 0.18f);
        c.spireMid   = glm::vec3(0.82f, 0.22f, 0.12f);
        c.spireBase  = glm::vec3(0.60f, 0.14f, 0.08f);
        break;

    case 4: // Autumn Copper Ochre
        c.tipCenter  = glm::vec3(0.94f, 0.66f, 0.18f);
        c.tipSide    = glm::vec3(0.86f, 0.54f, 0.14f);
        c.valley     = glm::vec3(0.62f, 0.30f, 0.08f);
        c.midGreen   = glm::vec3(0.78f, 0.44f, 0.12f);
        c.ridgeMid   = glm::vec3(0.90f, 0.58f, 0.16f);
        c.deepTeal   = glm::vec3(0.42f, 0.18f, 0.06f);
        c.underside  = glm::vec3(0.34f, 0.14f, 0.05f);
        c.spireTip   = glm::vec3(0.98f, 0.72f, 0.22f);
        c.spireMid   = glm::vec3(0.84f, 0.50f, 0.14f);
        c.spireBase  = glm::vec3(0.64f, 0.32f, 0.09f);
        break;

    case 0: // Summer Green (Direct match to reference image)
    default:
        c.tipCenter  = glm::vec3(0.58f, 0.90f, 0.22f);
        c.tipSide    = glm::vec3(0.54f, 0.86f, 0.20f);
        c.valley     = glm::vec3(0.25f, 0.64f, 0.16f);
        c.midGreen   = glm::vec3(0.20f, 0.60f, 0.16f);
        c.ridgeMid   = glm::vec3(0.42f, 0.78f, 0.19f);
        c.deepTeal   = glm::vec3(0.06f, 0.24f, 0.12f);
        c.underside  = glm::vec3(0.04f, 0.18f, 0.09f);
        c.spireTip   = glm::vec3(0.64f, 0.95f, 0.24f);
        c.spireMid   = glm::vec3(0.40f, 0.82f, 0.19f);
        c.spireBase  = glm::vec3(0.18f, 0.58f, 0.16f);
        break;
    }
    return c;
}

// Centerline function for leaning/curving trunks
glm::vec3 getCenterline(float y, const TreeParams& p)
{
    float normY = std::clamp(y / p.totalHeight, 0.0f, 1.0f);
    float factor = normY * normY;
    return glm::vec3(p.leanX * factor, y, p.leanZ * factor);
}

// Helper to append a triangle
void addTriangle(std::vector<unsigned int>& indices, unsigned int i0, unsigned int i1, unsigned int i2)
{
    indices.push_back(i0);
    indices.push_back(i1);
    indices.push_back(i2);
}

// Generates a stylized needle frond with creased central ridge and 3-5 serrated fingertips
void generateNeedleFrond(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec3& rootPos,
    float centerAngle,
    float arcSpan,
    float length,
    float droopFactor,
    float ridgeHeightFactor,
    const FoliageColors& colors,
    bool isUpperLayer)
{
    const int NUM_SPOKES = 7;
    const float spokeAnglesNorm[NUM_SPOKES] = { -0.50f, -0.32f, -0.16f, 0.0f, 0.16f, 0.32f, 0.50f };
    const float tipRadiiNorm[NUM_SPOKES]   = { 0.65f,  0.88f,  0.72f, 1.00f, 0.72f, 0.88f, 0.65f };

    unsigned int baseVertexIndex = static_cast<unsigned int>(vertices.size());

    // -----------------------------------------------------------------------
    // TOP SURFACE (Sunlit upward/outward facing)
    // -----------------------------------------------------------------------
    for (int band = 0; band < 4; ++band)
    {
        float bandT = (band == 0) ? 0.0f : (band == 1) ? 0.42f : (band == 2) ? 0.72f : 1.0f;

        for (int s = 0; s < NUM_SPOKES; ++s)
        {
            float spokeAngle = centerAngle + spokeAnglesNorm[s] * arcSpan;
            float spokeRadius = length * bandT;
            if (band == 3)
            {
                spokeRadius = length * tipRadiiNorm[s];
            }

            float rNorm = spokeRadius / (length > 0.001f ? length : 1.0f);
            float droop = droopFactor * (rNorm * rNorm) * length;

            float ridgeFactor = 1.0f - std::abs(spokeAnglesNorm[s]) * 2.0f;
            if (ridgeFactor < 0.0f) ridgeFactor = 0.0f;
            float ridgeElevation = ridgeHeightFactor * ridgeFactor * (1.0f - rNorm * 0.4f);

            glm::vec3 pos;
            pos.x = rootPos.x + std::cos(spokeAngle) * spokeRadius;
            pos.z = rootPos.z + std::sin(spokeAngle) * spokeRadius;
            pos.y = rootPos.y - droop + ridgeElevation;

            glm::vec3 normal;
            float normRadial = 0.35f;
            float normUp = 0.90f;
            float normTangent = spokeAnglesNorm[s] * -0.45f;
            normal.x = std::cos(spokeAngle) * normRadial - std::sin(spokeAngle) * normTangent;
            normal.z = std::sin(spokeAngle) * normRadial + std::cos(spokeAngle) * normTangent;
            normal.y = normUp;
            normal = glm::normalize(normal);

            glm::vec3 color;
            if (band == 0)
            {
                color = colors.deepTeal;
            }
            else if (band == 1)
            {
                color = (s == 3) ? colors.ridgeMid : colors.midGreen;
            }
            else if (band == 2)
            {
                color = (s == 3) ? glm::mix(colors.midGreen, colors.tipCenter, 0.45f) : colors.midGreen;
            }
            else // band == 3 (Tips & valleys)
            {
                if (s == 3)
                    color = colors.tipCenter;
                else if (s == 1 || s == 5)
                    color = colors.tipSide;
                else
                    color = colors.valley;
            }

            if (isUpperLayer)
            {
                color = glm::clamp(color * 1.08f, 0.0f, 1.0f);
            }

            vertices.push_back({ pos, normal, color });
        }
    }

    // Top surface triangle indices
    for (int band = 0; band < 3; ++band)
    {
        int bandStart = baseVertexIndex + band * NUM_SPOKES;
        int nextBandStart = bandStart + NUM_SPOKES;

        for (int s = 0; s < NUM_SPOKES - 1; ++s)
        {
            unsigned int v0 = bandStart + s;
            unsigned int v1 = bandStart + s + 1;
            unsigned int v2 = nextBandStart + s;
            unsigned int v3 = nextBandStart + s + 1;

            addTriangle(indices, v0, v2, v1);
            addTriangle(indices, v1, v2, v3);
        }
    }

    // -----------------------------------------------------------------------
    // UNDERSIDE SURFACE (Dark shadowed undertone with downward normals)
    // -----------------------------------------------------------------------
    unsigned int underBase = static_cast<unsigned int>(vertices.size());
    for (int band = 0; band < 4; ++band)
    {
        int srcBandStart = baseVertexIndex + band * NUM_SPOKES;
        for (int s = 0; s < NUM_SPOKES; ++s)
        {
            const Vertex& topV = vertices[srcBandStart + s];
            glm::vec3 underPos = topV.position - glm::vec3(0.0f, 0.035f, 0.0f);
            glm::vec3 underNorm = glm::vec3(-topV.normal.x * 0.4f, -0.92f, -topV.normal.z * 0.4f);
            underNorm = glm::normalize(underNorm);
            glm::vec3 underColor = glm::mix(colors.underside, topV.color, 0.20f);

            vertices.push_back({ underPos, underNorm, underColor });
        }
    }

    // Underside triangle indices
    for (int band = 0; band < 3; ++band)
    {
        int bandStart = underBase + band * NUM_SPOKES;
        int nextBandStart = bandStart + NUM_SPOKES;

        for (int s = 0; s < NUM_SPOKES - 1; ++s)
        {
            unsigned int v0 = bandStart + s;
            unsigned int v1 = bandStart + s + 1;
            unsigned int v2 = nextBandStart + s;
            unsigned int v3 = nextBandStart + s + 1;

            addTriangle(indices, v0, v1, v2);
            addTriangle(indices, v1, v3, v2);
        }
    }
}

// Generates a gnarled dead twig sticking out below the canopy
void generateDeadTwig(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec3& rootPos,
    float angle,
    float length,
    float droopAngleDeg)
{
    const glm::vec3 colTwigDark(0.24f, 0.15f, 0.08f);
    const glm::vec3 colTwigLight(0.35f, 0.23f, 0.14f);

    float droopRad = glm::radians(droopAngleDeg);
    float cosDroop = std::cos(droopRad);
    float sinDroop = std::sin(droopRad);

    glm::vec3 dir(std::cos(angle) * cosDroop, -sinDroop, std::sin(angle) * cosDroop);
    dir = glm::normalize(dir);

    glm::vec3 tipPos = rootPos + dir * length;
    tipPos.y -= 0.08f;

    unsigned int startIdx = static_cast<unsigned int>(vertices.size());
    const int SIDES = 4;
    float rBase = 0.055f;
    float rTip  = 0.015f;

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 side = glm::normalize(glm::cross(dir, up));
    glm::vec3 normUp = glm::normalize(glm::cross(side, dir));

    // Base ring
    for (int i = 0; i < SIDES; ++i)
    {
        float a = (float)i * (2.0f * M_PI / (float)SIDES);
        glm::vec3 offset = (side * std::cos(a) + normUp * std::sin(a)) * rBase;
        glm::vec3 norm = glm::normalize(offset);
        glm::vec3 col = (i % 2 == 0) ? colTwigLight : colTwigDark;
        vertices.push_back({ rootPos + offset, norm, col });
    }

    // Tip ring
    for (int i = 0; i < SIDES; ++i)
    {
        float a = (float)i * (2.0f * M_PI / (float)SIDES);
        glm::vec3 offset = (side * std::cos(a) + normUp * std::sin(a)) * rTip;
        glm::vec3 norm = glm::normalize(offset);
        glm::vec3 col = colTwigDark;
        vertices.push_back({ tipPos + offset, norm, col });
    }

    // Connect sides
    for (int i = 0; i < SIDES; ++i)
    {
        int next = (i + 1) % SIDES;
        unsigned int b0 = startIdx + i;
        unsigned int b1 = startIdx + next;
        unsigned int t0 = startIdx + SIDES + i;
        unsigned int t1 = startIdx + SIDES + next;

        addTriangle(indices, b0, t0, b1);
        addTriangle(indices, b1, t0, t1);
    }

    // Offshoot twiglet
    if (length > 0.6f)
    {
        glm::vec3 subRoot = rootPos + dir * (length * 0.55f);
        float subAngle = angle + 0.65f;
        glm::vec3 subDir(std::cos(subAngle) * 0.85f, -0.45f, std::sin(subAngle) * 0.85f);
        subDir = glm::normalize(subDir);
        glm::vec3 subTip = subRoot + subDir * (length * 0.38f);

        unsigned int subBase = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ subRoot + side * 0.02f, up, colTwigDark });
        vertices.push_back({ subRoot - side * 0.02f, -up, colTwigDark });
        vertices.push_back({ subTip, subDir, colTwigDark });
        addTriangle(indices, subBase, subBase + 1, subBase + 2);
    }
}

} // anonymous namespace

StylizedPineMesh::StylizedPineMesh(int variant, int paletteType)
{
    buildMesh(variant, paletteType);
}

void StylizedPineMesh::draw(const Shader& shader) const
{
    // 1. Draw trunk & dead twigs with realistic bark texture
    TextureManager::getInstance().setTextureMode(shader, TEX_BARK, 0.70f);
    m_trunkMesh.draw();

    // 2. Draw foliage needle boughs & crown spire with leaf-fiber texture
    TextureManager::getInstance().setTextureMode(shader, TEX_LEAF, 1.45f);
    m_foliageMesh.draw();

    // 3. Reset mode to TEX_NONE
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);
}

void StylizedPineMesh::draw() const
{
    m_trunkMesh.draw();
    m_foliageMesh.draw();
}

void StylizedPineMesh::buildMesh(int variant, int paletteType)
{
    TreeParams p = getParams(variant);
    FoliageColors fCol = getFoliageColors(paletteType);

    std::vector<Vertex> trunkVerts;
    std::vector<unsigned int> trunkIdx;
    trunkVerts.reserve(1000);
    trunkIdx.reserve(3000);

    std::vector<Vertex> foliageVerts;
    std::vector<unsigned int> foliageIdx;
    foliageVerts.reserve(4500);
    foliageIdx.reserve(13500);

    // -----------------------------------------------------------------------
    // 1. FLUTED TAPERED TRUNK WITH ROOT FLARE & BARK RIDGES
    // -----------------------------------------------------------------------
    const int TRUNK_RINGS = 16;
    const int TRUNK_SEGS  = 10;
    const glm::vec3 colBarkGroove(0.22f, 0.14f, 0.08f);
    const glm::vec3 colBarkRidge (0.38f, 0.25f, 0.15f);

    unsigned int trunkStartIdx = static_cast<unsigned int>(trunkVerts.size());

    for (int r = 0; r <= TRUNK_RINGS; ++r)
    {
        float ringT = (float)r / (float)TRUNK_RINGS;
        float y = ringT * p.totalHeight;

        glm::vec3 center = getCenterline(y, p);

        float baseR = p.trunkBaseRadius;
        float flare = 0.0f;
        if (y < 0.40f)
        {
            flare = 0.60f * std::exp(-y / 0.12f);
        }
        float radius = baseR * (1.0f - ringT * 0.85f + flare);
        if (radius < 0.05f) radius = 0.05f;

        for (int s = 0; s < TRUNK_SEGS; ++s)
        {
            float theta = (float)s * (2.0f * M_PI / (float)TRUNK_SEGS);

            float fluting = 1.0f + 0.08f * std::cos(5.0f * theta) + 0.04f * std::sin(9.0f * theta);
            float rFinal = radius * fluting;

            glm::vec3 pos;
            pos.x = center.x + std::cos(theta) * rFinal;
            pos.z = center.z + std::sin(theta) * rFinal;
            pos.y = y;

            glm::vec3 norm = glm::normalize(glm::vec3(std::cos(theta), 0.12f, std::sin(theta)));

            float ridgePattern = 0.5f + 0.5f * std::cos(5.0f * theta);
            glm::vec3 col = glm::mix(colBarkGroove, colBarkRidge, ridgePattern);

            trunkVerts.push_back({ pos, norm, col });
        }
    }

    for (int r = 0; r < TRUNK_RINGS; ++r)
    {
        int ringStart = trunkStartIdx + r * TRUNK_SEGS;
        int nextRingStart = ringStart + TRUNK_SEGS;

        for (int s = 0; s < TRUNK_SEGS; ++s)
        {
            int nextS = (s + 1) % TRUNK_SEGS;

            unsigned int v0 = ringStart + s;
            unsigned int v1 = ringStart + nextS;
            unsigned int v2 = nextRingStart + s;
            unsigned int v3 = nextRingStart + nextS;

            addTriangle(trunkIdx, v0, v2, v1);
            addTriangle(trunkIdx, v1, v2, v3);
        }
    }

    // -----------------------------------------------------------------------
    // 2. WITHERED DEAD LOWER TWIGS (Belonging to Trunk Mesh)
    // -----------------------------------------------------------------------
    float twigHeight = p.canopyStartHeight * 0.75f;
    glm::vec3 twigTrunkCenter = getCenterline(twigHeight, p);

    for (int i = 0; i < p.deadTwigCount; ++i)
    {
        float angle = (float)i * (2.0f * M_PI / (float)p.deadTwigCount) + 0.35f * std::sin((float)i * 2.3f);
        float len = p.deadTwigLength * (0.80f + 0.35f * std::cos((float)i * 1.7f));
        float droopDeg = 22.0f + 12.0f * std::sin((float)i * 1.9f);

        glm::vec3 root = twigTrunkCenter;
        root.x += std::cos(angle) * (p.trunkBaseRadius * 0.75f);
        root.z += std::sin(angle) * (p.trunkBaseRadius * 0.75f);
        root.y += ((float)i - (float)p.deadTwigCount * 0.5f) * 0.05f;

        generateDeadTwig(trunkVerts, trunkIdx, root, angle, len, droopDeg);
    }

    // -----------------------------------------------------------------------
    // 3. TIERED CONCENTRIC FOLIAGE BOUGHS (Belonging to Foliage Mesh)
    // -----------------------------------------------------------------------
    float canopyHeightSpan = p.totalHeight - 0.85f - p.canopyStartHeight;

    for (int t = 0; t < p.numTiers; ++t)
    {
        float tNorm = (float)t / (float)(p.numTiers > 1 ? p.numTiers - 1 : 1);
        float tierY = p.canopyStartHeight + tNorm * canopyHeightSpan;

        glm::vec3 tierCenter = getCenterline(tierY, p);

        float taper = 1.0f - std::pow(tNorm, p.canopyTaperPower);
        float tierRadius = p.maxCanopyRadius * taper;
        tierRadius *= (0.92f + 0.14f * std::sin((float)t * 2.1f));
        if (tierRadius < 0.45f) tierRadius = 0.45f;

        int numBoughs = std::max(5, (int)(8.0f - tNorm * 3.0f));
        float tierAngleOffset = (float)t * 1.18f + 0.15f * std::sin((float)t * 3.1f);
        float arcSpan = (2.0f * M_PI / (float)numBoughs) * 1.25f;

        for (int b = 0; b < numBoughs; ++b)
        {
            float centerAngle = tierAngleOffset + (float)b * (2.0f * M_PI / (float)numBoughs);

            float trunkR = p.trunkBaseRadius * (1.0f - (tierY / p.totalHeight) * 0.7f);
            glm::vec3 rootPos = tierCenter;
            rootPos.x += std::cos(centerAngle) * trunkR;
            rootPos.z += std::sin(centerAngle) * trunkR;
            rootPos.y += 0.04f * std::sin((float)b * 1.7f);

            float boughLength = tierRadius * (0.94f + 0.12f * std::cos((float)(b + t) * 1.4f));
            float droop = p.boughDroopFactor * (0.85f + 0.30f * (1.0f - tNorm));
            float ridgeH = 0.14f * boughLength;

            // Primary Lower Bough Layer
            generateNeedleFrond(
                foliageVerts, foliageIdx,
                rootPos,
                centerAngle,
                arcSpan,
                boughLength,
                droop,
                ridgeH,
                fCol,
                false
            );

            // Secondary Upper Bough Layer
            glm::vec3 upperRoot = rootPos + glm::vec3(0.0f, 0.10f + 0.04f * (1.0f - tNorm), 0.0f);
            generateNeedleFrond(
                foliageVerts, foliageIdx,
                upperRoot,
                centerAngle + 0.12f,
                arcSpan * 0.90f,
                boughLength * 0.82f,
                droop * 0.85f,
                ridgeH * 0.90f,
                fCol,
                true
            );
        }
    }

    // -----------------------------------------------------------------------
    // 4. APICAL POINTED CROWN SPIRE (Belonging to Foliage Mesh)
    // -----------------------------------------------------------------------
    float spireBaseY = p.totalHeight - 0.95f;
    float spireTopY  = p.totalHeight;
    glm::vec3 spireBaseCenter = getCenterline(spireBaseY, p);
    glm::vec3 spireTopCenter  = getCenterline(spireTopY, p);

    const int SPIRE_SEGS = 7;
    unsigned int spireBaseIdx = static_cast<unsigned int>(foliageVerts.size());

    float spireR = 0.38f;
    for (int i = 0; i < SPIRE_SEGS; ++i)
    {
        float angle = (float)i * (2.0f * M_PI / (float)SPIRE_SEGS);
        glm::vec3 pos;
        pos.x = spireBaseCenter.x + std::cos(angle) * spireR;
        pos.z = spireBaseCenter.z + std::sin(angle) * spireR;
        pos.y = spireBaseY;

        glm::vec3 norm = glm::normalize(glm::vec3(std::cos(angle), 0.45f, std::sin(angle)));
        foliageVerts.push_back({ pos, norm, fCol.spireBase });
    }

    for (int i = 0; i < SPIRE_SEGS; ++i)
    {
        float angle = (float)i * (2.0f * M_PI / (float)SPIRE_SEGS) + 0.2f;
        glm::vec3 pos;
        pos.x = spireBaseCenter.x + std::cos(angle) * (spireR * 0.45f);
        pos.z = spireBaseCenter.z + std::sin(angle) * (spireR * 0.45f);
        pos.y = spireBaseY + 0.52f;

        glm::vec3 norm = glm::normalize(glm::vec3(std::cos(angle), 0.65f, std::sin(angle)));
        foliageVerts.push_back({ pos, norm, fCol.spireMid });
    }

    unsigned int apexIdx = static_cast<unsigned int>(foliageVerts.size());
    foliageVerts.push_back({ spireTopCenter, glm::vec3(0.0f, 1.0f, 0.0f), fCol.spireTip });

    for (int i = 0; i < SPIRE_SEGS; ++i)
    {
        int next = (i + 1) % SPIRE_SEGS;
        unsigned int b0 = spireBaseIdx + i;
        unsigned int b1 = spireBaseIdx + next;
        unsigned int m0 = spireBaseIdx + SPIRE_SEGS + i;
        unsigned int m1 = spireBaseIdx + SPIRE_SEGS + next;

        addTriangle(foliageIdx, b0, m0, b1);
        addTriangle(foliageIdx, b1, m0, m1);
        addTriangle(foliageIdx, m0, apexIdx, m1);
    }

    // Initialize the two separate submeshes
    m_trunkMesh.init(trunkVerts, trunkIdx);
    m_foliageMesh.init(foliageVerts, foliageIdx);
}
