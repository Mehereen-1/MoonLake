#include "objects/TrainStation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

TrainStation::TrainStation(const glm::vec3& pos, float rotY, float scale)
    : position(pos), rotationY(rotY), scaleFactor(scale)
{
}

void TrainStation::draw(const Shader& shader,
                        const Cube& unitCube,
                        const Cylinder& unitCylinder,
                        const Sphere& unitSphere,
                        float currentTime) const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, position);
    root = glm::rotate(root, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::scale(root, glm::vec3(scaleFactor));

    // Natural material palettes
    const glm::vec3 stoneBase(0.32f, 0.30f, 0.26f);    // Weathered foundation curb
    const glm::vec3 woodDeck(0.42f, 0.29f, 0.18f);     // Cedar deck planks
    const glm::vec3 woodDeckAlt(0.38f, 0.26f, 0.16f);  // Alternating deck planks
    const glm::vec3 timberPost(0.26f, 0.16f, 0.10f);   // Dark oak posts & beams
    const glm::vec3 roofCedar(0.36f, 0.24f, 0.14f);    // Pitched cedar roof
    const glm::vec3 edgeSafety(0.55f, 0.44f, 0.22f);   // Platform safety curb strip
    const glm::vec3 warmGlow(1.0f, 0.78f, 0.38f);      // Lantern illumination
    const glm::vec3 brassTrim(0.85f, 0.70f, 0.32f);

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 12.0f);

    // =======================================================================
    // 1. Raised Boarding Platform
    // =======================================================================
    // A. Foundation curb
    glm::mat4 baseMat = glm::translate(root, glm::vec3(0.0f, 0.16f, 0.0f));
    baseMat = glm::scale(baseMat, glm::vec3(12.6f, 0.32f, 2.40f));
    shader.setMat4("model", baseMat);
    shader.setVec3("objectColor", stoneBase);
    unitCube.draw();

    // B. Wooden Deck Planks (Multiple planks running along platform surface)
    for (float px = -5.8f; px <= 5.8f; px += 0.45f)
    {
        glm::mat4 plank = glm::translate(root, glm::vec3(px, 0.33f, 0.0f));
        plank = glm::scale(plank, glm::vec3(0.42f, 0.04f, 2.36f));
        shader.setMat4("model", plank);
        shader.setVec3("objectColor", ((int)(px * 10.0f) % 2 == 0) ? woodDeck : woodDeckAlt);
        unitCube.draw();
    }

    // C. Trackside Platform Safety Edge Strip (Front edge facing train)
    glm::mat4 edgeMat = glm::translate(root, glm::vec3(0.0f, 0.34f, 1.18f));
    edgeMat = glm::scale(edgeMat, glm::vec3(12.6f, 0.05f, 0.14f));
    shader.setMat4("model", edgeMat);
    shader.setVec3("objectColor", edgeSafety);
    unitCube.draw();

    // D. Boarding Entrance Ramp & Timber Steps (Leading down toward walking path from dock)
    glm::mat4 rampMat = glm::translate(root, glm::vec3(0.0f, 0.14f, -1.45f));
    rampMat = glm::scale(rampMat, glm::vec3(3.20f, 0.18f, 0.70f));
    shader.setMat4("model", rampMat);
    shader.setVec3("objectColor", woodDeck);
    unitCube.draw();

    glm::mat4 rampStep = glm::translate(root, glm::vec3(0.0f, 0.06f, -1.95f));
    rampStep = glm::scale(rampStep, glm::vec3(3.40f, 0.10f, 0.60f));
    shader.setMat4("model", rampStep);
    shader.setVec3("objectColor", woodDeckAlt);
    unitCube.draw();

    // =======================================================================
    // 2. Rustic Timber Shelter / Pavilion (Posts, Trusses & Pitched Roof)
    // =======================================================================
    // 6 Sturdy Upright Timber Posts
    const float postX[3] = { -4.6f, 0.0f, 4.6f };
    const float postZ[2] = { -0.92f, 0.92f };

    for (int ix = 0; ix < 3; ++ix)
    {
        for (int iz = 0; iz < 2; ++iz)
        {
            glm::mat4 post = glm::translate(root, glm::vec3(postX[ix], 1.55f, postZ[iz]));
            post = glm::scale(post, glm::vec3(0.20f, 2.45f, 0.20f));
            shader.setMat4("model", post);
            shader.setVec3("objectColor", timberPost);
            unitCube.draw();

            // Diagonal Knee-Braces at top of posts
            glm::mat4 brace = glm::translate(root, glm::vec3(postX[ix], 2.50f, postZ[iz] * 0.70f));
            brace = glm::rotate(brace, glm::radians(iz == 0 ? 35.0f : -35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            brace = glm::scale(brace, glm::vec3(0.12f, 0.42f, 0.12f));
            shader.setMat4("model", brace);
            unitCube.draw();
        }
    }

    // Long Horizontal Ridge & Eaves Beams
    for (int iz = 0; iz < 2; ++iz)
    {
        glm::mat4 eaves = glm::translate(root, glm::vec3(0.0f, 2.72f, postZ[iz]));
        eaves = glm::scale(eaves, glm::vec3(10.2f, 0.16f, 0.18f));
        shader.setMat4("model", eaves);
        shader.setVec3("objectColor", timberPost);
        unitCube.draw();
    }

    // Central Roof Ridge Beam
    glm::mat4 ridgeBeam = glm::translate(root, glm::vec3(0.0f, 3.25f, 0.0f));
    ridgeBeam = glm::scale(ridgeBeam, glm::vec3(10.4f, 0.18f, 0.18f));
    shader.setMat4("model", ridgeBeam);
    unitCube.draw();

    // Pitched Wooden Roof (Left slope & Right slope with overhang)
    // Front roof slope (toward train)
    glm::mat4 roofFront = glm::translate(root, glm::vec3(0.0f, 3.02f, 0.62f));
    roofFront = glm::rotate(roofFront, glm::radians(24.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    roofFront = glm::scale(roofFront, glm::vec3(10.6f, 0.08f, 1.48f));
    shader.setMat4("model", roofFront);
    shader.setVec3("objectColor", roofCedar);
    unitCube.draw();

    // Back roof slope (toward path)
    glm::mat4 roofBack = glm::translate(root, glm::vec3(0.0f, 3.02f, -0.62f));
    roofBack = glm::rotate(roofBack, glm::radians(-24.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    roofBack = glm::scale(roofBack, glm::vec3(10.6f, 0.08f, 1.48f));
    shader.setMat4("model", roofBack);
    shader.setVec3("objectColor", roofCedar);
    unitCube.draw();

    // =======================================================================
    // 3. Station Name Sign ("MEADOW STATION")
    // =======================================================================
    // Suspended timber sign board
    glm::mat4 signBoard = glm::translate(root, glm::vec3(0.0f, 2.45f, -0.96f));
    signBoard = glm::scale(signBoard, glm::vec3(2.60f, 0.44f, 0.08f));
    shader.setMat4("model", signBoard);
    shader.setVec3("objectColor", timberPost);
    unitCube.draw();

    // Lettering plate
    glm::mat4 signPlate = glm::translate(root, glm::vec3(0.0f, 2.45f, -1.01f));
    signPlate = glm::scale(signPlate, glm::vec3(2.40f, 0.32f, 0.04f));
    shader.setMat4("model", signPlate);
    shader.setVec3("objectColor", glm::vec3(0.92f, 0.88f, 0.78f));
    unitCube.draw();

    // Brass accent rim
    glm::mat4 signTrim = glm::translate(root, glm::vec3(0.0f, 2.45f, -1.02f));
    signTrim = glm::scale(signTrim, glm::vec3(2.48f, 0.36f, 0.02f));
    shader.setMat4("model", signTrim);
    shader.setVec3("objectColor", brassTrim);
    unitCube.draw();

    // =======================================================================
    // 4. Passenger Waiting Benches on the Platform
    // =======================================================================
    const float benchX[2] = { -2.8f, 2.8f };
    for (int b = 0; b < 2; ++b)
    {
        // Bench legs
        const float legZ[2] = { -0.50f, -0.15f };
        for (int lz = 0; lz < 2; ++lz)
        {
            glm::mat4 leg = glm::translate(root, glm::vec3(benchX[b] - 0.65f, 0.52f, legZ[lz]));
            leg = glm::scale(leg, glm::vec3(0.08f, 0.34f, 0.08f));
            shader.setMat4("model", leg);
            shader.setVec3("objectColor", timberPost);
            unitCube.draw();

            glm::mat4 leg2 = glm::translate(root, glm::vec3(benchX[b] + 0.65f, 0.52f, legZ[lz]));
            leg2 = glm::scale(leg2, glm::vec3(0.08f, 0.34f, 0.08f));
            shader.setMat4("model", leg2);
            unitCube.draw();
        }

        // Bench wooden seat slats
        glm::mat4 seat = glm::translate(root, glm::vec3(benchX[b], 0.68f, -0.32f));
        seat = glm::scale(seat, glm::vec3(1.50f, 0.06f, 0.44f));
        shader.setMat4("model", seat);
        shader.setVec3("objectColor", woodDeck);
        unitCube.draw();

        // Bench backrest
        glm::mat4 backrest = glm::translate(root, glm::vec3(benchX[b], 0.95f, -0.52f));
        backrest = glm::rotate(backrest, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        backrest = glm::scale(backrest, glm::vec3(1.50f, 0.36f, 0.06f));
        shader.setMat4("model", backrest);
        shader.setVec3("objectColor", woodDeck);
        unitCube.draw();
    }

    // =======================================================================
    // 5. Back Platform Railing & Hanging Flower Planter Boxes
    // =======================================================================
    // Back safety railing along path side
    const float railSegs[2] = { -3.2f, 3.2f };
    for (int r = 0; r < 2; ++r)
    {
        glm::mat4 rail = glm::translate(root, glm::vec3(railSegs[r], 0.78f, -1.05f));
        rail = glm::scale(rail, glm::vec3(3.40f, 0.08f, 0.08f));
        shader.setMat4("model", rail);
        shader.setVec3("objectColor", timberPost);
        unitCube.draw();

        // Hanging wooden flower planter box
        glm::mat4 box = glm::translate(root, glm::vec3(railSegs[r], 0.72f, -1.16f));
        box = glm::scale(box, glm::vec3(1.80f, 0.22f, 0.24f));
        shader.setMat4("model", box);
        shader.setVec3("objectColor", woodDeckAlt);
        unitCube.draw();

        // Flower blooms inside the box (Magenta & pink cosmos)
        const glm::vec3 bloomCol1(0.88f, 0.22f, 0.54f);
        const glm::vec3 bloomCol2(0.94f, 0.50f, 0.70f);
        for (float fx = -0.65f; fx <= 0.65f; fx += 0.32f)
        {
            glm::mat4 fl = glm::translate(root, glm::vec3(railSegs[r] + fx, 0.88f, -1.16f));
            fl = glm::scale(fl, glm::vec3(0.14f, 0.12f, 0.14f));
            shader.setMat4("model", fl);
            shader.setVec3("objectColor", ((int)(fx * 10.0f) % 2 == 0) ? bloomCol1 : bloomCol2);
            unitSphere.draw();
        }
    }

    // =======================================================================
    // 6. Platform Lantern Posts (Warm Welcoming Illumination)
    // =======================================================================
    const float lantX[2] = { -5.8f, 5.8f };
    for (int l = 0; l < 2; ++l)
    {
        // Dark iron post
        glm::mat4 post = glm::translate(root, glm::vec3(lantX[l], 1.25f, 0.0f));
        post = glm::scale(post, glm::vec3(0.10f, 1.80f, 0.10f));
        shader.setMat4("model", post);
        shader.setVec3("objectColor", timberPost);
        unitCylinder.draw();

        // Lantern head bracket
        glm::mat4 bracket = glm::translate(root, glm::vec3(lantX[l], 2.18f, 0.0f));
        bracket = glm::scale(bracket, glm::vec3(0.24f, 0.28f, 0.24f));
        shader.setMat4("model", bracket);
        unitCube.draw();

        // Glowing warm spherical glass globe
        shader.setBool("isEmissive", true);
        float flicker = 1.0f + 0.06f * std::sin(currentTime * 8.0f + (float)l * 2.5f);
        shader.setVec3("emissionColor", warmGlow * (1.6f * flicker));
        shader.setVec3("objectColor", warmGlow);

        glm::mat4 globe = glm::translate(root, glm::vec3(lantX[l], 2.18f, 0.0f));
        globe = glm::scale(globe, glm::vec3(0.16f, 0.18f, 0.16f));
        shader.setMat4("model", globe);
        unitSphere.draw();

        shader.setBool("isEmissive", false);
    }
}
