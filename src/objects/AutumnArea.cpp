#include "objects/AutumnArea.h"
#include "texture/TextureManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

AutumnArea::AutumnArea()
{
    initTrees();
    initPath();
    initLeaves();
    initStumpsAndLogs();
    initBench();
    initDock();
    initLanterns();
    initRocks();
}

void AutumnArea::initTrees()
{
    // Natural realistic branching autumn trees framing the lakeside path and meadow
    trees = {
        AutumnTree(glm::vec3( 5.2f, -0.68f, 25.5f), 1.35f,  25.0f, 0), // Vibrant Amber Orange
        AutumnTree(glm::vec3(15.2f, -0.58f, 28.5f), 1.45f, -35.0f, 1), // Golden Saffron Yellow
        AutumnTree(glm::vec3(-3.5f, -0.52f, 31.0f), 1.50f,  60.0f, 2), // Deep Crimson Red
        AutumnTree(glm::vec3(13.5f, -0.50f, 33.5f), 1.40f, -15.0f, 3)  // Warm Copper Ochre
    };
}

void AutumnArea::initPath()
{
    // Walking path starts at the lake dock (X = 10.5m, Z = 22.2m),
    // curves gently through the flower meadow to the Train Station boarding ramp (X = 8.5m, Z = 27.5m),
    // with a scenic spur connecting to the lakeside bench (X = 12.8m, Z = 24.5m).
    pathSegments = {
        // Segment 1: Starting at the foot of the dock
        { glm::vec3(10.5f, -0.73f, 22.3f), glm::vec3(1.8f, 0.05f, 1.4f),  -8.0f },
        // Segment 2: Stepping up toward the flower meadow
        { glm::vec3(10.0f, -0.71f, 23.6f), glm::vec3(1.8f, 0.05f, 1.4f), -16.0f },
        // Segment 3: Beside bench path junction
        { glm::vec3( 9.4f, -0.68f, 24.9f), glm::vec3(1.8f, 0.05f, 1.4f), -20.0f },
        // Segment 4: Approaching station entrance
        { glm::vec3( 8.8f, -0.66f, 26.2f), glm::vec3(2.0f, 0.05f, 1.4f), -14.0f },
        // Segment 5: Connecting directly onto station boarding ramp
        { glm::vec3( 8.5f, -0.63f, 27.5f), glm::vec3(2.6f, 0.06f, 1.4f),  -8.0f },
        // Segment 6: Spur leading to the scenic lake overlook bench
        { glm::vec3(11.4f, -0.69f, 24.2f), glm::vec3(1.6f, 0.05f, 1.2f),  35.0f }
    };
}

void AutumnArea::initLeaves()
{
    // Dead autumn leaves removed in favor of lush blooming wildflowers
    leafClusters.clear();
}

void AutumnArea::initStumpsAndLogs()
{
    // Tree stumps: short cylinders with slightly larger radius near trees
    stumps = {
        { glm::vec3( 4.5f, -0.68f, 26.5f),  18.0f, 0.38f, 0.40f },
        { glm::vec3(12.5f, -0.57f, 29.2f), -35.0f, 0.42f, 0.45f },
        { glm::vec3( -2.8f, -0.51f, 32.0f),  55.0f, 0.35f, 0.38f }
    };

    // Fallen logs: horizontal cylinders rotated ~90 degrees beside the path
    logs = {
        // Log 1: Beside the path near the bench area (length 2.4m, radius 0.22m)
        { glm::vec3(10.8f, -0.64f, 27.0f), -28.0f, 90.0f, glm::vec3(0.38f, 2.4f, 0.38f) },
        // Log 2: In the mid-forest edge (length 2.0m, radius 0.20m)
        { glm::vec3( 4.8f, -0.58f, 28.5f),  65.0f, 88.0f, glm::vec3(0.34f, 2.0f, 0.34f) }
    };
}

void AutumnArea::initBench()
{
    // Bench placed beside the walking path, overlooking the lake
    // Position: X = 13.0m, Z = 24.6m (elevated shore facing toward Lake / campsite: rotY ~ 185 deg)
    benchPos = glm::vec3(12.8f, -0.66f, 24.5f);
    benchRotY = 195.0f;
    benchScale = 1.0f;
}

void AutumnArea::initDock()
{
    // Small wooden dock extending slightly into the lake
    // Lake water level: Y = -1.18m. Shoreline at X ~ 10.5m is Z ~ 22.0m.
    // Dock extends from Z = 22.2m outward into water toward Z = 18.2m (length ~ 4.0m, width ~ 2.2m)
    // Boat navigable boundary allows Z up to 16.5m, so boat can comfortably approach this dock!
    dockPos = glm::vec3(10.5f, -1.05f, 20.2f);
    dockRotY = 0.0f;
}

void AutumnArea::initLanterns()
{
    // A few warm lanterns along path and near the dock
    lanterns = {
        // Lantern 1: At the dock entrance (welcoming warm beacon)
        { glm::vec3( 9.2f, -0.72f, 21.6f),  15.0f, 1.25f },
        // Lantern 2: Beside the wooden bench overlooking the lake
        { glm::vec3(11.5f, -0.67f, 24.0f), -20.0f, 1.15f },
        // Lantern 3: Deep in the walking path forest bend
        { glm::vec3( 6.2f, -0.58f, 29.0f),  45.0f, 1.20f }
    };
}

void AutumnArea::initRocks()
{
    // Small rocks scattered along the path and near the dock/waterline
    rocks = {
        { glm::vec3(11.8f, -0.72f, 21.8f), glm::vec3(0.45f, 0.25f, 0.40f),  45.0f },
        { glm::vec3( 9.6f, -0.70f, 23.8f), glm::vec3(0.38f, 0.20f, 0.35f), 120.0f },
        { glm::vec3(12.0f, -0.66f, 25.8f), glm::vec3(0.48f, 0.28f, 0.42f), -30.0f },
        { glm::vec3( 6.8f, -0.62f, 27.2f), glm::vec3(0.40f, 0.22f, 0.36f),  85.0f },
        { glm::vec3( 8.2f, -0.54f, 31.0f), glm::vec3(0.50f, 0.30f, 0.45f), -70.0f }
    };
}

glm::vec3 AutumnArea::getPrimaryLanternLightPos() const
{
    // Return the world position of the dock lantern glowing sphere
    if (!lanterns.empty())
    {
        return lanterns[0].pos + glm::vec3(0.0f, lanterns[0].height, 0.0f);
    }
    return glm::vec3(9.2f, 0.5f, 21.6f);
}

glm::vec3 AutumnArea::getPrimaryLanternLightColor() const
{
    // Warm golden lantern illumination
    return glm::vec3(1.0f, 0.72f, 0.32f);
}

// ---------------------------------------------------------------------------
// Hierarchical Bench Model:
// Bench
// ├── Legs (4 vertical cuboids)
// ├── Seat (horizontal timber planks)
// └── Backrest (angled horizontal support and slats)
// M_bench_part = M_root * T_offset * R_rotation * S_scale
// ---------------------------------------------------------------------------
void AutumnArea::drawBench(const Shader& shader, const Cube& unitCube) const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, benchPos);
    root = glm::rotate(root, glm::radians(benchRotY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::scale(root, glm::vec3(benchScale));

    const glm::vec3 woodDark(0.24f, 0.15f, 0.09f); // Dark timber frame
    const glm::vec3 woodMain(0.38f, 0.24f, 0.14f); // Warm cedar slats

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 16.0f);

    auto drawPart = [&](const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& col, float rotX = 0.0f)
    {
        glm::mat4 m = root;
        m = glm::translate(m, offset);
        if (rotX != 0.0f) m = glm::rotate(m, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::scale(m, scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", col);
        unitCube.draw();
    };

    // 1. Legs (4 corner posts)
    const float legW = 0.08f;
    const float legH = 0.48f;
    drawPart(glm::vec3(-0.75f, legH * 0.5f,  0.22f), glm::vec3(legW, legH, legW), woodDark);
    drawPart(glm::vec3( 0.75f, legH * 0.5f,  0.22f), glm::vec3(legW, legH, legW), woodDark);
    drawPart(glm::vec3(-0.75f, legH * 0.5f, -0.22f), glm::vec3(legW, legH, legW), woodDark);
    drawPart(glm::vec3( 0.75f, legH * 0.5f, -0.22f), glm::vec3(legW, legH, legW), woodDark);

    // 2. Crossbeams (Under seat support)
    drawPart(glm::vec3(0.0f, 0.44f,  0.22f), glm::vec3(1.58f, 0.06f, 0.06f), woodDark);
    drawPart(glm::vec3(0.0f, 0.44f, -0.22f), glm::vec3(1.58f, 0.06f, 0.06f), woodDark);
    drawPart(glm::vec3(-0.75f, 0.44f, 0.0f), glm::vec3(0.06f, 0.06f, 0.44f), woodDark);
    drawPart(glm::vec3( 0.75f, 0.44f, 0.0f), glm::vec3(0.06f, 0.06f, 0.44f), woodDark);

    // 3. Seat (3 horizontal timber slats)
    for (int i = -1; i <= 1; ++i)
    {
        drawPart(glm::vec3(0.0f, 0.50f, i * 0.16f), glm::vec3(1.68f, 0.045f, 0.13f), woodMain);
    }

    // 4. Backrest Uprights & Slats (slanted backward ~12 deg)
    drawPart(glm::vec3(-0.75f, 0.78f, -0.26f), glm::vec3(0.07f, 0.55f, 0.07f), woodDark, -12.0f);
    drawPart(glm::vec3( 0.75f, 0.78f, -0.26f), glm::vec3(0.07f, 0.55f, 0.07f), woodDark, -12.0f);

    // Horizontal backrest slats
    drawPart(glm::vec3(0.0f, 0.72f, -0.26f), glm::vec3(1.68f, 0.10f, 0.035f), woodMain, -12.0f);
    drawPart(glm::vec3(0.0f, 0.90f, -0.30f), glm::vec3(1.68f, 0.10f, 0.035f), woodMain, -12.0f);
}

// ---------------------------------------------------------------------------
// Small Wooden Dock Model:
// Dock extending into the water
// ├── Vertical Support Posts (Cylinders / Cubes into water)
// ├── Stringer Beams (Horizontal frame)
// └── Deck Planks (Cuboids spanning the walkway)
// ---------------------------------------------------------------------------
void AutumnArea::drawDock(const Shader& shader, const Cube& unitCube) const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, dockPos);
    root = glm::rotate(root, glm::radians(dockRotY), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 woodPilings(0.20f, 0.14f, 0.09f); // Weathered submerged pilings
    const glm::vec3 woodStringer(0.26f, 0.17f, 0.11f); // Frame beams
    const glm::vec3 woodDeck(0.38f, 0.26f, 0.16f);     // Autumn wooden planks

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 14.0f);

    auto drawPart = [&](const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& col)
    {
        glm::mat4 m = root;
        m = glm::translate(m, offset);
        m = glm::scale(m, scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", col);
        unitCube.draw();
    };

    // 1. Vertical Piling Posts (into the water, Y from -1.18m to -0.45m)
    // Left & right pilings at shore (Z = +1.6m), mid (Z = 0.0m), and pier head (Z = -1.6m)
    const float postW = 0.18f;
    const float postH = 1.10f;
    drawPart(glm::vec3(-0.95f, 0.0f,  1.6f), glm::vec3(postW, postH, postW), woodPilings);
    drawPart(glm::vec3( 0.95f, 0.0f,  1.6f), glm::vec3(postW, postH, postW), woodPilings);
    drawPart(glm::vec3(-0.95f, 0.0f,  0.0f), glm::vec3(postW, postH, postW), woodPilings);
    drawPart(glm::vec3( 0.95f, 0.0f,  0.0f), glm::vec3(postW, postH, postW), woodPilings);
    drawPart(glm::vec3(-0.95f, 0.0f, -1.6f), glm::vec3(postW, postH, postW), woodPilings);
    drawPart(glm::vec3( 0.95f, 0.0f, -1.6f), glm::vec3(postW, postH, postW), woodPilings);

    // 2. Horizontal Stringer Beams (running longitudinally along Z)
    drawPart(glm::vec3(-0.90f, 0.44f, 0.0f), glm::vec3(0.14f, 0.15f, 3.8f), woodStringer);
    drawPart(glm::vec3( 0.90f, 0.44f, 0.0f), glm::vec3(0.14f, 0.15f, 3.8f), woodStringer);
    // Center stringer
    drawPart(glm::vec3( 0.00f, 0.44f, 0.0f), glm::vec3(0.14f, 0.15f, 3.8f), woodStringer);

    // 3. Wooden Dock Deck Planks (Transverse planks along X)
    // Span from Z = +1.8m to -1.8m
    for (float pz = -1.8f; pz <= 1.8f; pz += 0.34f)
    {
        // Alternate subtle shade for hand-crafted plank appearance
        float tint = 1.0f + 0.08f * std::sin(pz * 7.0f);
        drawPart(glm::vec3(0.0f, 0.54f, pz), glm::vec3(2.15f, 0.065f, 0.28f), woodDeck * tint);
    }

    // 4. Mooring Bollards / Cleats at pier head
    drawPart(glm::vec3(-0.95f, 0.65f, -1.7f), glm::vec3(0.14f, 0.20f, 0.14f), woodPilings);
    drawPart(glm::vec3( 0.95f, 0.65f, -1.7f), glm::vec3(0.14f, 0.20f, 0.14f), woodPilings);
}

// ---------------------------------------------------------------------------
// Warm Lantern Model:
// Lantern
// ├── Wooden Post (Cylinder / Cuboid)
// ├── Metal Frame & Cap (Cuboid & Pyramid)
// └── Glowing Light Core (Warm Emissive Sphere with subtle flicker)
// ---------------------------------------------------------------------------
void AutumnArea::drawLantern(const Shader& shader,
                             const Cube& unitCube,
                             const Cylinder& unitCylinder,
                             const Sphere& unitSphere,
                             const AutumnLantern& lantern,
                             float currentTime) const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, lantern.pos);
    root = glm::rotate(root, glm::radians(lantern.rotY), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 woodPost(0.24f, 0.15f, 0.09f);
    const glm::vec3 ironDark(0.15f, 0.15f, 0.17f);
    const glm::vec3 warmGlow(1.0f, 0.74f, 0.35f);

    // 1. Post (Cylinder from Y = 0 to lantern.height)
    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 16.0f);
    {
        glm::mat4 pMat = glm::scale(root, glm::vec3(0.12f, lantern.height, 0.12f));
        shader.setMat4("model", pMat);
        shader.setVec3("objectColor", woodPost);
        unitCylinder.draw();
    }

    // Crossarm bracket
    {
        glm::mat4 armMat = glm::translate(root, glm::vec3(0.12f, lantern.height - 0.05f, 0.0f));
        armMat = glm::scale(armMat, glm::vec3(0.28f, 0.05f, 0.05f));
        shader.setMat4("model", armMat);
        shader.setVec3("objectColor", ironDark);
        unitCube.draw();
    }

    // 2. Lantern Cage & Frame (Iron top cap and bottom base)
    glm::vec3 lanternCenter(0.24f, lantern.height - 0.18f, 0.0f);
    {
        // Bottom base
        glm::mat4 baseMat = glm::translate(root, lanternCenter - glm::vec3(0.0f, 0.14f, 0.0f));
        baseMat = glm::scale(baseMat, glm::vec3(0.18f, 0.03f, 0.18f));
        shader.setMat4("model", baseMat);
        shader.setVec3("objectColor", ironDark);
        unitCube.draw();

        // Top roof cap
        glm::mat4 capMat = glm::translate(root, lanternCenter + glm::vec3(0.0f, 0.14f, 0.0f));
        capMat = glm::scale(capMat, glm::vec3(0.20f, 0.04f, 0.20f));
        shader.setMat4("model", capMat);
        shader.setVec3("objectColor", ironDark);
        unitCube.draw();
    }

    // 3. Glowing Inner Sphere (Subtle warm harmonic pulsation)
    float flicker = 0.92f + 0.08f * std::sin(currentTime * 5.0f + lantern.pos.x * 3.0f);
    shader.setBool("isEmissive", true);
    shader.setVec3("emissionColor", warmGlow * flicker * 1.5f);
    {
        glm::mat4 coreMat = glm::translate(root, lanternCenter);
        coreMat = glm::scale(coreMat, glm::vec3(0.10f));
        shader.setMat4("model", coreMat);
        shader.setVec3("objectColor", warmGlow);
        unitSphere.draw();
    }
    shader.setBool("isEmissive", false);
}

void AutumnArea::draw(const Shader& shader,
                      const Cube& unitCube,
                      const Cylinder& unitCylinder,
                      const Sphere& unitSphere,
                      const Pyramid& unitPyramid,
                      const Rock& rockMesh,
                      float currentTime) const
{
    // 1. Draw Distant Treeline crowning the southern mountain horizon
    treeline.draw(shader, unitCube, unitCylinder, unitSphere, unitPyramid);

    // 2. Draw Lush Blooming Wildflower Field (Magenta, Pink, Lavender, White cosmos)
    wildflowers.draw(shader);

    // 3. Draw Lakeside Meadow Boarding Station
    station.draw(shader, unitCube, unitCylinder, unitSphere, currentTime);

    // 4. Draw Scenic Vintage Tourist Train & Promenade Track (Untextured materials)
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE);
    train.draw(shader, unitCube, unitCylinder, unitSphere, currentTime);

    // 5. Draw Autumn Trees (if any)
    for (const auto& tree : trees)
    {
        tree.draw(shader, unitCylinder, unitSphere, unitPyramid);
    }

    // 2. Draw Walking Path (Slightly curved earthy stone slabs)
    const glm::vec3 pathStone(0.36f, 0.30f, 0.22f); // Earthy cobblestone
    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 8.0f);
    for (const auto& p : pathSegments)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, p.pos);
        m = glm::rotate(m, glm::radians(p.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, p.scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", pathStone);
        unitCube.draw();
    }

    // 3. Draw Fallen Leaves (Grouped flat clusters)
    shader.setFloat("shininess", 16.0f);
    for (const auto& lc : leafClusters)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, lc.pos);
        m = glm::rotate(m, glm::radians(lc.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, lc.scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", lc.color);
        unitCube.draw();
    }

    // 4. Draw Tree Stumps
    const glm::vec3 stumpBark(0.26f, 0.16f, 0.10f);
    const glm::vec3 stumpRing(0.38f, 0.28f, 0.16f);
    for (const auto& st : stumps)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, st.pos);
        m = glm::rotate(m, glm::radians(st.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(st.radius * 2.0f, st.height, st.radius * 2.0f));
        shader.setMat4("model", m);
        shader.setVec3("objectColor", stumpBark);
        unitCylinder.draw();
    }

    // 5. Draw Fallen Logs (Horizontal cylinders rotated ~90 degrees)
    const glm::vec3 logBark(0.24f, 0.15f, 0.09f);
    for (const auto& log : logs)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, log.pos);
        m = glm::rotate(m, glm::radians(log.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::rotate(m, glm::radians(log.rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, log.scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", logBark);
        unitCylinder.draw();
    }

    // 6. Draw Small Rocks
    shader.setFloat("shininess", 28.0f);
    for (const auto& r : rocks)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, r.pos);
        m = glm::rotate(m, glm::radians(r.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, r.scale);
        shader.setMat4("model", m);
        shader.setVec3("objectColor", glm::vec3(0.28f, 0.26f, 0.24f));
        rockMesh.draw();
    }

    // 7. Draw Wooden Bench
    drawBench(shader, unitCube);

    // 8. Draw Small Dock
    drawDock(shader, unitCube);

    // 9. Draw Lanterns
    for (const auto& lant : lanterns)
    {
        drawLantern(shader, unitCube, unitCylinder, unitSphere, lant, currentTime);
    }
}
