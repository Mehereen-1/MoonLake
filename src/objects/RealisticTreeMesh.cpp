#include "objects/RealisticTreeMesh.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace {

#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

// Tree Palette structure for foliage and bark
struct TreeColors {
    glm::vec3 barkDark;
    glm::vec3 barkLight;
    glm::vec3 foliageUpper;
    glm::vec3 foliageMid;
    glm::vec3 foliageLower;
    glm::vec3 foliageTip;
};

TreeColors getColors(int paletteType)
{
    TreeColors c;
    c.barkDark  = glm::vec3(0.24f, 0.15f, 0.09f);
    c.barkLight = glm::vec3(0.34f, 0.22f, 0.13f);

    switch (paletteType % 5)
    {
    case 1: // Autumn Amber Orange
        c.foliageUpper = glm::vec3(0.96f, 0.55f, 0.10f);
        c.foliageMid   = glm::vec3(0.88f, 0.42f, 0.08f);
        c.foliageLower = glm::vec3(0.68f, 0.28f, 0.06f);
        c.foliageTip   = glm::vec3(0.98f, 0.68f, 0.15f);
        break;
    case 2: // Autumn Golden Saffron
        c.foliageUpper = glm::vec3(0.98f, 0.84f, 0.16f);
        c.foliageMid   = glm::vec3(0.92f, 0.70f, 0.12f);
        c.foliageLower = glm::vec3(0.76f, 0.50f, 0.08f);
        c.foliageTip   = glm::vec3(1.00f, 0.90f, 0.28f);
        break;
    case 3: // Autumn Crimson Red
        c.foliageUpper = glm::vec3(0.86f, 0.25f, 0.13f);
        c.foliageMid   = glm::vec3(0.76f, 0.18f, 0.10f);
        c.foliageLower = glm::vec3(0.56f, 0.12f, 0.07f);
        c.foliageTip   = glm::vec3(0.92f, 0.35f, 0.18f);
        break;
    case 4: // Autumn Copper Ochre
        c.foliageUpper = glm::vec3(0.90f, 0.58f, 0.15f);
        c.foliageMid   = glm::vec3(0.80f, 0.44f, 0.12f);
        c.foliageLower = glm::vec3(0.60f, 0.30f, 0.08f);
        c.foliageTip   = glm::vec3(0.95f, 0.70f, 0.20f);
        break;
    case 0: // Lush Summer Green (Realistic botanical temperate tree matching user's image)
    default:
        c.foliageUpper = glm::vec3(0.32f, 0.68f, 0.16f); // Sunlit spring lime crown
        c.foliageMid   = glm::vec3(0.18f, 0.52f, 0.13f); // Rich chlorophyll summer leaf
        c.foliageLower = glm::vec3(0.10f, 0.36f, 0.08f); // Deep forest emerald underside
        c.foliageTip   = glm::vec3(0.44f, 0.82f, 0.22f); // Vibrant fresh tip growth
        break;
    }
    return c;
}

// Procedural Mesh Builder Helper
struct MeshBuilder {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    void addCylinder(const glm::vec3& p1, const glm::vec3& p2, float r1, float r2, int segments,
                     const glm::vec3& col1, const glm::vec3& col2)
    {
        glm::vec3 v = p2 - p1;
        float len = glm::length(v);
        if (len < 0.0001f) return;
        glm::vec3 d = v / len;

        glm::vec3 up = (std::abs(d.y) < 0.95f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 u = glm::normalize(glm::cross(d, up));
        glm::vec3 w = glm::normalize(glm::cross(d, u));

        unsigned int baseIdx = static_cast<unsigned int>(vertices.size());

        for (int i = 0; i <= segments; ++i)
        {
            float angle = (i % segments) * (2.0f * M_PI / segments);
            glm::vec3 radial = std::cos(angle) * u + std::sin(angle) * w;

            glm::vec3 norm = radial;
            glm::vec3 vA = p1 + r1 * radial;
            glm::vec3 vB = p2 + r2 * radial;

            vertices.push_back({ vA, norm, col1 });
            vertices.push_back({ vB, norm, col2 });
        }

        for (int i = 0; i < segments; ++i)
        {
            unsigned int b1 = baseIdx + i * 2;
            unsigned int t1 = baseIdx + i * 2 + 1;
            unsigned int b2 = baseIdx + (i + 1) * 2;
            unsigned int t2 = baseIdx + (i + 1) * 2 + 1;

            indices.push_back(b1);
            indices.push_back(t1);
            indices.push_back(b2);

            indices.push_back(b2);
            indices.push_back(t1);
            indices.push_back(t2);
        }
    }

    void addJoint(const glm::vec3& p, float radius, const glm::vec3& col)
    {
        const int stacks = 4;
        const int sectors = 6;
        unsigned int startIdx = static_cast<unsigned int>(vertices.size());

        for (int i = 0; i <= stacks; ++i)
        {
            float V = (float)i / (float)stacks;
            float phi = V * M_PI;

            for (int j = 0; j <= sectors; ++j)
            {
                float U = (float)j / (float)sectors;
                float theta = U * (2.0f * M_PI);

                float x = std::cos(theta) * std::sin(phi);
                float y = std::cos(phi);
                float z = std::sin(theta) * std::sin(phi);

                glm::vec3 norm(x, y, z);
                glm::vec3 pos = p + norm * radius;
                vertices.push_back({ pos, norm, col });
            }
        }

        for (int i = 0; i < stacks; ++i)
        {
            for (int j = 0; j < sectors; ++j)
            {
                unsigned int first  = startIdx + i * (sectors + 1) + j;
                unsigned int second = startIdx + (i + 1) * (sectors + 1) + j;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(first + 1);
                indices.push_back(second);
                indices.push_back(second + 1);
            }
        }
    }

    void addLeafCluster(const glm::vec3& center, float radius, int bladeCount, const TreeColors& colors)
    {
        // -------------------------------------------------------------------
        // 1. Organic Volumetric Foliage Core (Provides density and soft ambient shading)
        // -------------------------------------------------------------------
        const int coreSectors = 6;
        const int coreStacks  = 3;
        unsigned int coreStart = static_cast<unsigned int>(vertices.size());
        float coreRadius = radius * 0.72f;

        for (int i = 0; i <= coreStacks; ++i)
        {
            float V = (float)i / (float)coreStacks;
            float phi = V * M_PI;

            for (int j = 0; j <= coreSectors; ++j)
            {
                float U = (float)j / (float)coreSectors;
                float theta = U * (2.0f * M_PI);

                float x = std::cos(theta) * std::sin(phi);
                float y = std::cos(phi);
                float z = std::sin(theta) * std::sin(phi);

                glm::vec3 norm = glm::normalize(glm::vec3(x, y, z));
                // Subtle organic perturbation
                float jitter = 0.88f + 0.24f * std::sin(theta * 3.0f + phi * 2.0f);
                glm::vec3 pos = center + norm * (coreRadius * jitter);

                // Vertical color gradient
                float t = (norm.y + 1.0f) * 0.5f;
                glm::vec3 col = glm::mix(colors.foliageLower, colors.foliageUpper, t);

                vertices.push_back({ pos, norm, col });
            }
        }

        for (int i = 0; i < coreStacks; ++i)
        {
            for (int j = 0; j < coreSectors; ++j)
            {
                unsigned int f = coreStart + i * (coreSectors + 1) + j;
                unsigned int s = coreStart + (i + 1) * (coreSectors + 1) + j;

                indices.push_back(f);
                indices.push_back(s);
                indices.push_back(f + 1);

                indices.push_back(f + 1);
                indices.push_back(s);
                indices.push_back(s + 1);
            }
        }

        // -------------------------------------------------------------------
        // 2. Creased Leaf Blades (Multi-angled leaf cards with specular ridges)
        // -------------------------------------------------------------------
        float leafLength = radius * 0.65f;
        float leafWidth  = radius * 0.42f;
        float crease     = leafWidth * 0.25f;

        for (int b = 0; b < bladeCount; ++b)
        {
            // Spherical distribution with upward & outward bias
            float phi = std::acos(1.0f - 2.0f * ((float)b + 0.5f) / (float)bladeCount);
            float theta = b * 2.3999632f; // Golden ratio angle

            float dx = std::cos(theta) * std::sin(phi);
            float dy = std::abs(std::cos(phi)) * 0.8f + 0.2f; // upward bias
            float dz = std::sin(theta) * std::sin(phi);

            glm::vec3 dir = glm::normalize(glm::vec3(dx, dy, dz));
            glm::vec3 bladeCenter = center + dir * (radius * 0.58f);

            glm::vec3 upRef = (std::abs(dir.y) < 0.92f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            glm::vec3 rSide = glm::normalize(glm::cross(dir, upRef));
            glm::vec3 lNorm = glm::normalize(glm::cross(dir, rSide));

            glm::vec3 vRoot  = bladeCenter - dir * (leafLength * 0.5f);
            glm::vec3 vTip   = bladeCenter + dir * (leafLength * 0.5f);
            glm::vec3 vLeft  = bladeCenter - rSide * (leafWidth * 0.5f) - lNorm * crease;
            glm::vec3 vRight = bladeCenter + rSide * (leafWidth * 0.5f) - lNorm * crease;

            glm::vec3 nLeft  = glm::normalize(glm::cross(vTip - vRoot, vLeft - vRoot));
            glm::vec3 nRight = glm::normalize(glm::cross(vRight - vRoot, vTip - vRoot));

            glm::vec3 cRoot = colors.foliageMid * 0.88f;
            glm::vec3 cSide = colors.foliageUpper;
            glm::vec3 cTip  = colors.foliageTip;

            unsigned int bIdx = static_cast<unsigned int>(vertices.size());
            vertices.push_back({ vRoot,  nLeft,  cRoot }); // 0
            vertices.push_back({ vLeft,  nLeft,  cSide }); // 1
            vertices.push_back({ vTip,   nLeft,  cTip  }); // 2
            vertices.push_back({ vRight, nRight, cSide }); // 3

            // Double-sided leaf rendering for full visibility from all camera angles
            // Left facet
            indices.push_back(bIdx + 0);
            indices.push_back(bIdx + 1);
            indices.push_back(bIdx + 2);
            indices.push_back(bIdx + 0);
            indices.push_back(bIdx + 2);
            indices.push_back(bIdx + 1);

            // Right facet
            indices.push_back(bIdx + 0);
            indices.push_back(bIdx + 2);
            indices.push_back(bIdx + 3);
            indices.push_back(bIdx + 0);
            indices.push_back(bIdx + 3);
            indices.push_back(bIdx + 2);
        }
    }
};

} // anonymous namespace

RealisticTreeMesh::RealisticTreeMesh(int variant, int paletteType)
{
    buildMesh(variant, paletteType);
}

void RealisticTreeMesh::buildMesh(int variant, int paletteType)
{
    MeshBuilder b;
    TreeColors colors = getColors(paletteType);

    // -----------------------------------------------------------------------
    // VARIANT 0: Direct Match to User's Reference Image
    // Features:
    //   - Segmented brown trunk with natural curves and root flare.
    //   - Prominent lower-right bough extending horizontally/downward then curling up.
    //   - Ascending curved primary boughs (left and central crown).
    //   - Multi-tier recursive branching into sub-branches and twigs.
    //   - DENSE, LUSH FOLIAGE across tips, branches, and crown!
    // -----------------------------------------------------------------------
    if ((variant % 4) == 0)
    {
        // 1. Trunk Base & Lower Section
        glm::vec3 pBase(0.0f, 0.0f, 0.0f);
        glm::vec3 pTrunk1(0.0f, 0.95f, 0.0f);
        glm::vec3 pTrunk2(-0.04f, 1.72f, 0.03f);

        b.addCylinder(pBase, pTrunk1, 0.36f, 0.28f, 8, colors.barkDark * 0.9f, colors.barkLight);
        b.addJoint(pTrunk1, 0.29f, colors.barkDark);
        b.addCylinder(pTrunk1, pTrunk2, 0.28f, 0.24f, 8, colors.barkLight, colors.barkDark);
        b.addJoint(pTrunk2, 0.25f, colors.barkDark);

        // -------------------------------------------------------------------
        // 2. Prominent Lower Right Bough (Matching the user's reference image!)
        // -------------------------------------------------------------------
        glm::vec3 pR1(0.65f, 1.62f, 0.12f);
        glm::vec3 pR2(1.35f, 1.50f, 0.18f);
        glm::vec3 pR3(1.85f, 1.56f, 0.14f);

        b.addCylinder(pTrunk2, pR1, 0.20f, 0.16f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(pR1, 0.17f, colors.barkDark);
        b.addCylinder(pR1, pR2, 0.16f, 0.13f, 7, colors.barkLight, colors.barkDark);
        b.addJoint(pR2, 0.14f, colors.barkDark);
        b.addCylinder(pR2, pR3, 0.13f, 0.11f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pR3, 0.12f, colors.barkDark);

        // Intermediate foliage on main right bough
        b.addLeafCluster(pR2 + glm::vec3(0.0f, 0.25f, 0.05f), 0.48f, 6, colors);

        // Lower-right branch fork A: Hanging downward branch
        glm::vec3 pR_Down1(2.25f, 1.25f, 0.22f);
        glm::vec3 pR_Down2(2.55f, 0.85f, 0.25f);
        b.addCylinder(pR3, pR_Down1, 0.10f, 0.08f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pR_Down1, 0.09f, colors.barkDark);
        b.addCylinder(pR_Down1, pR_Down2, 0.08f, 0.06f, 5, colors.barkDark, colors.barkLight);
        b.addJoint(pR_Down2, 0.07f, colors.barkDark);

        // Twigs for Downward Branch
        glm::vec3 pR_DTwig1(2.75f, 0.52f, 0.20f);
        glm::vec3 pR_DTwig2(2.95f, 0.95f, 0.32f);
        glm::vec3 pR_DTwig3(2.45f, 0.70f, 0.12f);
        b.addCylinder(pR_Down2, pR_DTwig1, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pR_Down2, pR_DTwig2, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pR_Down2, pR_DTwig3, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);

        // Leaf clusters on downward branch
        b.addLeafCluster(pR_DTwig1, 0.50f, 8, colors);
        b.addLeafCluster(pR_DTwig2, 0.52f, 8, colors);
        b.addLeafCluster(pR_DTwig3, 0.46f, 7, colors);
        b.addLeafCluster(pR_Down1 + glm::vec3(0.05f, 0.20f, 0.0f), 0.44f, 6, colors);

        // Lower-right branch fork B: Curling upward branch
        glm::vec3 pR_Up1(2.15f, 1.95f, 0.10f);
        glm::vec3 pR_Up2(2.45f, 2.35f, 0.05f);
        b.addCylinder(pR3, pR_Up1, 0.10f, 0.08f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pR_Up1, 0.09f, colors.barkDark);
        b.addCylinder(pR_Up1, pR_Up2, 0.08f, 0.06f, 5, colors.barkDark, colors.barkLight);
        b.addJoint(pR_Up2, 0.07f, colors.barkDark);

        // Twigs for Upward Branch
        glm::vec3 pR_UTwig1(2.78f, 2.65f, 0.02f);
        glm::vec3 pR_UTwig2(2.55f, 2.52f, 0.22f);
        glm::vec3 pR_UTwig3(2.25f, 2.75f, -0.10f);
        b.addCylinder(pR_Up2, pR_UTwig1, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pR_Up2, pR_UTwig2, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pR_Up2, pR_UTwig3, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);

        // Leaf clusters on upward branch
        b.addLeafCluster(pR_UTwig1, 0.52f, 8, colors);
        b.addLeafCluster(pR_UTwig2, 0.50f, 8, colors);
        b.addLeafCluster(pR_UTwig3, 0.48f, 7, colors);
        b.addLeafCluster(pR_Up1 + glm::vec3(0.0f, 0.22f, -0.05f), 0.45f, 6, colors);

        // -------------------------------------------------------------------
        // 3. Main Trunk Continuing Upward
        // -------------------------------------------------------------------
        glm::vec3 pTrunk3(-0.08f, 2.45f, -0.04f);
        glm::vec3 pTrunk4(-0.14f, 3.10f, -0.02f);
        b.addCylinder(pTrunk2, pTrunk3, 0.22f, 0.19f, 8, colors.barkDark, colors.barkLight);
        b.addJoint(pTrunk3, 0.20f, colors.barkDark);
        b.addCylinder(pTrunk3, pTrunk4, 0.19f, 0.17f, 7, colors.barkLight, colors.barkDark);
        b.addJoint(pTrunk4, 0.18f, colors.barkDark);

        // -------------------------------------------------------------------
        // 4. Primary Left Bough (Curving left and ascending)
        // -------------------------------------------------------------------
        glm::vec3 pL1(-0.55f, 3.55f, 0.12f);
        glm::vec3 pL2(-1.05f, 3.85f, 0.18f);
        b.addCylinder(pTrunk4, pL1, 0.15f, 0.13f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(pL1, 0.14f, colors.barkDark);
        b.addCylinder(pL1, pL2, 0.13f, 0.11f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pL2, 0.12f, colors.barkDark);

        b.addLeafCluster(pL1 + glm::vec3(0.0f, 0.25f, 0.0f), 0.52f, 7, colors);

        // Left Sub-branch Outer
        glm::vec3 pL_Out1(-1.45f, 4.25f, 0.22f);
        glm::vec3 pL_Out2(-1.75f, 4.60f, 0.25f);
        b.addCylinder(pL2, pL_Out1, 0.10f, 0.08f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pL_Out1, 0.09f, colors.barkDark);
        b.addCylinder(pL_Out1, pL_Out2, 0.08f, 0.06f, 5, colors.barkLight, colors.barkDark);
        b.addJoint(pL_Out2, 0.07f, colors.barkDark);

        glm::vec3 pL_OutTwig1(-1.98f, 4.90f, 0.30f);
        glm::vec3 pL_OutTwig2(-1.62f, 5.08f, 0.15f);
        glm::vec3 pL_OutTwig3(-2.08f, 4.48f, 0.10f);
        b.addCylinder(pL_Out2, pL_OutTwig1, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pL_Out2, pL_OutTwig2, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pL_Out2, pL_OutTwig3, 0.05f, 0.035f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pL_OutTwig1, 0.56f, 8, colors);
        b.addLeafCluster(pL_OutTwig2, 0.58f, 9, colors);
        b.addLeafCluster(pL_OutTwig3, 0.52f, 8, colors);
        b.addLeafCluster(pL_Out1 + glm::vec3(0.10f, 0.20f, -0.05f), 0.50f, 7, colors);

        // Left Sub-branch Upward
        glm::vec3 pL_Up1(-1.10f, 4.45f, 0.05f);
        glm::vec3 pL_Up2(-1.25f, 4.95f, 0.00f);
        b.addCylinder(pL2, pL_Up1, 0.09f, 0.07f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pL_Up1, 0.08f, colors.barkDark);
        b.addCylinder(pL_Up1, pL_Up2, 0.07f, 0.05f, 5, colors.barkDark, colors.barkLight);
        b.addJoint(pL_Up2, 0.06f, colors.barkDark);

        glm::vec3 pL_UpTwig1(-1.42f, 5.35f, 0.05f);
        glm::vec3 pL_UpTwig2(-1.05f, 5.42f, -0.15f);
        glm::vec3 pL_UpTwig3(-0.90f, 5.12f, 0.20f);
        b.addCylinder(pL_Up2, pL_UpTwig1, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pL_Up2, pL_UpTwig2, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pL_Up2, pL_UpTwig3, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pL_UpTwig1, 0.60f, 9, colors);
        b.addLeafCluster(pL_UpTwig2, 0.58f, 9, colors);
        b.addLeafCluster(pL_UpTwig3, 0.54f, 8, colors);
        b.addLeafCluster(pL_Up1, 0.52f, 7, colors);

        // Left Sub-branch Back/Lateral
        glm::vec3 pL_Bk1(-1.15f, 3.75f, -0.25f);
        glm::vec3 pL_Bk2(-1.40f, 4.15f, -0.45f);
        b.addCylinder(pL2, pL_Bk1, 0.08f, 0.065f, 5, colors.barkDark, colors.barkLight);
        b.addJoint(pL_Bk1, 0.075f, colors.barkDark);
        b.addCylinder(pL_Bk1, pL_Bk2, 0.065f, 0.05f, 5, colors.barkLight, colors.barkDark);
        b.addJoint(pL_Bk2, 0.06f, colors.barkDark);

        glm::vec3 pL_BkTwig1(-1.65f, 4.52f, -0.55f);
        glm::vec3 pL_BkTwig2(-1.25f, 4.62f, -0.35f);
        b.addCylinder(pL_Bk2, pL_BkTwig1, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pL_Bk2, pL_BkTwig2, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pL_BkTwig1, 0.55f, 8, colors);
        b.addLeafCluster(pL_BkTwig2, 0.52f, 8, colors);

        // -------------------------------------------------------------------
        // 5. Primary Center & Right Boughs (Forming the rich crown canopy)
        // -------------------------------------------------------------------
        glm::vec3 pC1(0.20f, 3.65f, -0.08f);
        glm::vec3 pC2(0.42f, 4.15f, -0.12f);
        b.addCylinder(pTrunk4, pC1, 0.15f, 0.13f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(pC1, 0.14f, colors.barkDark);
        b.addCylinder(pC1, pC2, 0.13f, 0.11f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pC2, 0.12f, colors.barkDark);

        b.addLeafCluster(pC1 + glm::vec3(-0.05f, 0.22f, 0.05f), 0.55f, 7, colors);

        // Center Apex Branch (Crown Top)
        glm::vec3 pC_Apex1(0.20f, 4.65f, -0.05f);
        glm::vec3 pC_Apex2(0.15f, 5.15f, 0.00f);
        b.addCylinder(pC2, pC_Apex1, 0.10f, 0.08f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pC_Apex1, 0.09f, colors.barkDark);
        b.addCylinder(pC_Apex1, pC_Apex2, 0.08f, 0.06f, 5, colors.barkLight, colors.barkDark);
        b.addJoint(pC_Apex2, 0.07f, colors.barkDark);

        glm::vec3 pC_ATwig1(0.00f, 5.62f, 0.05f);
        glm::vec3 pC_ATwig2(0.35f, 5.68f, -0.10f);
        glm::vec3 pC_ATwig3(0.25f, 5.38f, 0.25f);
        glm::vec3 pC_ATwig4(-0.15f, 5.32f, -0.15f);
        b.addCylinder(pC_Apex2, pC_ATwig1, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Apex2, pC_ATwig2, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Apex2, pC_ATwig3, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Apex2, pC_ATwig4, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pC_ATwig1, 0.65f, 10, colors);
        b.addLeafCluster(pC_ATwig2, 0.65f, 10, colors);
        b.addLeafCluster(pC_ATwig3, 0.60f, 9,  colors);
        b.addLeafCluster(pC_ATwig4, 0.58f, 8,  colors);
        b.addLeafCluster(pC_Apex1,  0.55f, 8,  colors);

        // Right Upper Crown Branch
        glm::vec3 pC_Rt1(0.82f, 4.55f, -0.18f);
        glm::vec3 pC_Rt2(1.12f, 4.95f, -0.22f);
        b.addCylinder(pC2, pC_Rt1, 0.09f, 0.075f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pC_Rt1, 0.085f, colors.barkDark);
        b.addCylinder(pC_Rt1, pC_Rt2, 0.075f, 0.055f, 5, colors.barkLight, colors.barkDark);
        b.addJoint(pC_Rt2, 0.065f, colors.barkDark);

        glm::vec3 pC_RtTwig1(1.42f, 5.32f, -0.25f);
        glm::vec3 pC_RtTwig2(1.18f, 5.42f, 0.00f);
        glm::vec3 pC_RtTwig3(1.50f, 4.92f, -0.10f);
        b.addCylinder(pC_Rt2, pC_RtTwig1, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Rt2, pC_RtTwig2, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Rt2, pC_RtTwig3, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pC_RtTwig1, 0.62f, 9, colors);
        b.addLeafCluster(pC_RtTwig2, 0.60f, 9, colors);
        b.addLeafCluster(pC_RtTwig3, 0.56f, 8, colors);
        b.addLeafCluster(pC_Rt1,     0.52f, 7, colors);

        // Forward Upper Crown Branch
        glm::vec3 pC_Fwd1(0.42f, 4.45f, 0.28f);
        glm::vec3 pC_Fwd2(0.58f, 4.90f, 0.42f);
        b.addCylinder(pC2, pC_Fwd1, 0.09f, 0.07f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pC_Fwd1, 0.08f, colors.barkDark);
        b.addCylinder(pC_Fwd1, pC_Fwd2, 0.07f, 0.05f, 5, colors.barkDark, colors.barkLight);
        b.addJoint(pC_Fwd2, 0.06f, colors.barkDark);

        glm::vec3 pC_FwdTwig1(0.74f, 5.32f, 0.50f);
        glm::vec3 pC_FwdTwig2(0.38f, 5.28f, 0.35f);
        b.addCylinder(pC_Fwd2, pC_FwdTwig1, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(pC_Fwd2, pC_FwdTwig2, 0.045f, 0.03f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pC_FwdTwig1, 0.60f, 9, colors);
        b.addLeafCluster(pC_FwdTwig2, 0.58f, 8, colors);
        b.addLeafCluster(pC_Fwd1,     0.50f, 7, colors);

        // -------------------------------------------------------------------
        // 6. Lush Crown Canopy Bridging Masses (Ensures full, voluminous foliage canopy)
        // -------------------------------------------------------------------
        b.addLeafCluster(glm::vec3(-0.45f, 4.95f,  0.08f), 0.68f, 9,  colors);
        b.addLeafCluster(glm::vec3( 0.48f, 5.05f, -0.06f), 0.68f, 9,  colors);
        b.addLeafCluster(glm::vec3(-0.10f, 5.48f,  0.02f), 0.72f, 10, colors);
        b.addLeafCluster(glm::vec3(-0.65f, 4.60f, -0.20f), 0.58f, 8,  colors);
        b.addLeafCluster(glm::vec3( 0.05f, 4.35f,  0.22f), 0.60f, 8,  colors);
    }
    // -----------------------------------------------------------------------
    // VARIANT 1: Majestic Tall Canopy Spreading Tree
    // -----------------------------------------------------------------------
    else if ((variant % 4) == 1)
    {
        glm::vec3 p0(0.0f, 0.0f, 0.0f);
        glm::vec3 p1(0.02f, 1.15f, -0.02f);
        glm::vec3 p2(0.0f, 2.10f, 0.0f);

        b.addCylinder(p0, p1, 0.38f, 0.30f, 8, colors.barkDark * 0.9f, colors.barkLight);
        b.addJoint(p1, 0.31f, colors.barkDark);
        b.addCylinder(p1, p2, 0.30f, 0.25f, 8, colors.barkLight, colors.barkDark);
        b.addJoint(p2, 0.26f, colors.barkDark);

        // Three symmetric main boughs
        for (int i = 0; i < 3; ++i)
        {
            float rot = i * (2.0f * M_PI / 3.0f);
            float dirX = std::cos(rot);
            float dirZ = std::sin(rot);

            glm::vec3 b1 = p2 + glm::vec3(dirX * 0.75f, 0.95f, dirZ * 0.75f);
            glm::vec3 b2 = b1 + glm::vec3(dirX * 0.85f, 0.85f, dirZ * 0.85f);
            b.addCylinder(p2, b1, 0.18f, 0.14f, 7, colors.barkDark, colors.barkLight);
            b.addJoint(b1, 0.15f, colors.barkDark);
            b.addCylinder(b1, b2, 0.14f, 0.10f, 6, colors.barkLight, colors.barkDark);
            b.addJoint(b2, 0.11f, colors.barkDark);

            b.addLeafCluster(b1 + glm::vec3(0.0f, 0.25f, 0.0f), 0.55f, 7, colors);

            // Sub-branches
            glm::vec3 t1 = b2 + glm::vec3(dirX * 0.55f, 0.65f, dirZ * 0.25f);
            glm::vec3 t2 = b2 + glm::vec3(-dirZ * 0.45f, 0.70f, dirX * 0.45f);
            glm::vec3 t3 = b2 + glm::vec3(dirX * 0.35f, 0.85f, -dirZ * 0.25f);

            b.addCylinder(b2, t1, 0.08f, 0.045f, 5, colors.barkDark, colors.barkLight);
            b.addCylinder(b2, t2, 0.08f, 0.045f, 5, colors.barkDark, colors.barkLight);
            b.addCylinder(b2, t3, 0.08f, 0.045f, 5, colors.barkDark, colors.barkLight);

            b.addLeafCluster(t1, 0.62f, 9, colors);
            b.addLeafCluster(t2, 0.60f, 9, colors);
            b.addLeafCluster(t3, 0.64f, 9, colors);
        }

        // Central Crown Apex
        glm::vec3 pApex(0.0f, 3.40f, 0.0f);
        glm::vec3 pApexTop(0.0f, 4.45f, 0.0f);
        b.addCylinder(p2, pApex, 0.18f, 0.12f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pApex, 0.13f, colors.barkDark);
        b.addCylinder(pApex, pApexTop, 0.12f, 0.06f, 5, colors.barkLight, colors.barkDark);

        b.addLeafCluster(pApexTop, 0.75f, 10, colors);
        b.addLeafCluster(pApexTop + glm::vec3(0.0f, 0.45f, 0.0f), 0.68f, 9, colors);
        b.addLeafCluster(glm::vec3( 0.40f, 4.10f,  0.30f), 0.65f, 8, colors);
        b.addLeafCluster(glm::vec3(-0.40f, 4.10f, -0.30f), 0.65f, 8, colors);
    }
    // -----------------------------------------------------------------------
    // VARIANT 2: Graceful Sweeping / Weeping Boughs Tree
    // -----------------------------------------------------------------------
    else if ((variant % 4) == 2)
    {
        glm::vec3 p0(0.0f, 0.0f, 0.0f);
        glm::vec3 p1(0.12f, 1.05f, 0.05f);
        glm::vec3 p2(0.35f, 2.05f, 0.10f);
        glm::vec3 p3(0.45f, 2.95f, 0.08f);

        b.addCylinder(p0, p1, 0.35f, 0.28f, 8, colors.barkDark * 0.9f, colors.barkLight);
        b.addJoint(p1, 0.29f, colors.barkDark);
        b.addCylinder(p1, p2, 0.28f, 0.23f, 8, colors.barkLight, colors.barkDark);
        b.addJoint(p2, 0.24f, colors.barkDark);
        b.addCylinder(p2, p3, 0.23f, 0.17f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(p3, 0.18f, colors.barkDark);

        // Lateral Low Branch sweeping left
        glm::vec3 pLeft1(-0.65f, 2.10f, -0.15f);
        glm::vec3 pLeft2(-1.35f, 2.30f, -0.22f);
        glm::vec3 pLeftHang(-1.75f, 1.80f, -0.28f);
        b.addCylinder(p2, pLeft1, 0.15f, 0.12f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pLeft1, 0.13f, colors.barkDark);
        b.addCylinder(pLeft1, pLeft2, 0.12f, 0.09f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(pLeft2, 0.10f, colors.barkDark);
        b.addCylinder(pLeft2, pLeftHang, 0.08f, 0.045f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(pLeftHang, 0.58f, 8, colors);
        b.addLeafCluster(pLeft2 + glm::vec3(0.0f, 0.25f, 0.0f), 0.52f, 7, colors);

        // Upper Crown Fork
        glm::vec3 pCrownA(0.15f, 3.80f, 0.25f);
        glm::vec3 pCrownB(0.85f, 3.70f, -0.10f);
        b.addCylinder(p3, pCrownA, 0.14f, 0.09f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pCrownA, 0.10f, colors.barkDark);
        b.addCylinder(p3, pCrownB, 0.14f, 0.09f, 6, colors.barkDark, colors.barkLight);
        b.addJoint(pCrownB, 0.10f, colors.barkDark);

        // Twigs & foliage
        glm::vec3 tA1(-0.10f, 4.45f, 0.35f);
        glm::vec3 tA2( 0.40f, 4.55f, 0.15f);
        glm::vec3 tB1( 1.25f, 4.35f, -0.25f);
        glm::vec3 tB2( 0.75f, 4.45f, -0.35f);

        b.addCylinder(pCrownA, tA1, 0.07f, 0.04f, 5, colors.barkLight, colors.barkDark);
        b.addCylinder(pCrownA, tA2, 0.07f, 0.04f, 5, colors.barkLight, colors.barkDark);
        b.addCylinder(pCrownB, tB1, 0.07f, 0.04f, 5, colors.barkLight, colors.barkDark);
        b.addCylinder(pCrownB, tB2, 0.07f, 0.04f, 5, colors.barkLight, colors.barkDark);

        b.addLeafCluster(tA1, 0.62f, 9, colors);
        b.addLeafCluster(tA2, 0.65f, 9, colors);
        b.addLeafCluster(tB1, 0.62f, 9, colors);
        b.addLeafCluster(tB2, 0.60f, 8, colors);

        b.addLeafCluster(glm::vec3(0.35f, 4.60f, 0.05f), 0.72f, 10, colors);
        b.addLeafCluster(glm::vec3(0.50f, 4.10f, 0.05f), 0.64f, 8,  colors);
    }
    // -----------------------------------------------------------------------
    // VARIANT 3: Twin-Stem Woodland Oak
    // -----------------------------------------------------------------------
    else
    {
        glm::vec3 p0(0.0f, 0.0f, 0.0f);
        glm::vec3 pSplit(0.0f, 1.25f, 0.0f);
        b.addCylinder(p0, pSplit, 0.42f, 0.32f, 8, colors.barkDark * 0.9f, colors.barkLight);
        b.addJoint(pSplit, 0.34f, colors.barkDark);

        // Stem 1 (Left forward)
        glm::vec3 s1A(-0.45f, 2.30f, 0.20f);
        glm::vec3 s1B(-0.75f, 3.35f, 0.30f);
        b.addCylinder(pSplit, s1A, 0.22f, 0.17f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(s1A, 0.18f, colors.barkDark);
        b.addCylinder(s1A, s1B, 0.17f, 0.12f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(s1B, 0.13f, colors.barkDark);

        glm::vec3 s1_t1(-1.15f, 4.05f, 0.40f);
        glm::vec3 s1_t2(-0.55f, 4.25f, 0.15f);
        b.addCylinder(s1B, s1_t1, 0.08f, 0.04f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(s1B, s1_t2, 0.08f, 0.04f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(s1_t1, 0.65f, 9, colors);
        b.addLeafCluster(s1_t2, 0.68f, 9, colors);
        b.addLeafCluster(s1A + glm::vec3(0.0f, 0.30f, 0.0f), 0.55f, 7, colors);

        // Stem 2 (Right backward)
        glm::vec3 s2A(0.48f, 2.35f, -0.18f);
        glm::vec3 s2B(0.85f, 3.40f, -0.28f);
        b.addCylinder(pSplit, s2A, 0.22f, 0.17f, 7, colors.barkDark, colors.barkLight);
        b.addJoint(s2A, 0.18f, colors.barkDark);
        b.addCylinder(s2A, s2B, 0.17f, 0.12f, 6, colors.barkLight, colors.barkDark);
        b.addJoint(s2B, 0.13f, colors.barkDark);

        glm::vec3 s2_t1(1.25f, 4.10f, -0.38f);
        glm::vec3 s2_t2(0.65f, 4.30f, -0.10f);
        b.addCylinder(s2B, s2_t1, 0.08f, 0.04f, 5, colors.barkDark, colors.barkLight);
        b.addCylinder(s2B, s2_t2, 0.08f, 0.04f, 5, colors.barkDark, colors.barkLight);

        b.addLeafCluster(s2_t1, 0.65f, 9, colors);
        b.addLeafCluster(s2_t2, 0.68f, 9, colors);
        b.addLeafCluster(s2A + glm::vec3(0.0f, 0.30f, 0.0f), 0.55f, 7, colors);

        // Central Canopy Bridge
        b.addLeafCluster(glm::vec3(0.05f, 4.20f, 0.05f), 0.74f, 10, colors);
        b.addLeafCluster(glm::vec3(0.00f, 4.65f, 0.00f), 0.70f, 10, colors);
    }

    setupMesh(b.vertices, b.indices);
}
