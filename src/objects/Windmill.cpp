#include "objects/Windmill.h"
#include "MotionSystem.h"
#include <glm/gtc/matrix_transform.hpp>

Windmill::Windmill(const glm::vec3& pos, float rotY, float scale)
    : position(pos), rotationY(rotY), scaleFactor(scale)
{
}

void Windmill::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Windmill::setRotation(float rotY)
{
    rotationY = rotY;
}

void Windmill::setScale(float scale)
{
    scaleFactor = scale;
}

const glm::vec3& Windmill::getPosition() const
{
    return position;
}

float Windmill::getRotation() const
{
    return rotationY;
}

float Windmill::getScale() const
{
    return scaleFactor;
}

glm::mat4 Windmill::getRootMatrix() const
{
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(scaleFactor));
    return m;
}

void Windmill::drawCube(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
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

void Windmill::drawPyramid(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
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

void Windmill::drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                          const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                          bool emissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
        shader.setVec3("emissionColor", emitColor);

    unitSphere.draw();
}

void Windmill::draw(const Shader& shader,
                    const Cube& unitCube,
                    const Pyramid& unitPyramid,
                    const Sphere& unitSphere,
                    const WindmillMotionState& motionState) const
{
    glm::mat4 rootMat = getRootMatrix();

    // 1. Foundation Plinth & Tapered Tower Shaft
    drawBaseAndTower(shader, unitCube, unitPyramid, rootMat);

    // 2. Wraparound Observation Balcony & Rooftop Cap
    drawBalconyAndCap(shader, unitCube, unitPyramid, unitSphere, rootMat);

    // 3. Rotating Rotor Hub & 4 Aerodynamic Sail Blades (Driven by motionState)
    drawRotorAndSails(shader, unitCube, unitPyramid, unitSphere, rootMat, motionState.rotationAngle);

    // Reset shader state
    shader.setBool("isEmissive", false);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}

void Windmill::draw(const Shader& shader,
                    const Cube& unitCube,
                    const Pyramid& unitPyramid,
                    const Sphere& unitSphere) const
{
    WindmillMotionState dummyState{ 0.0f, 0.0f };
    draw(shader, unitCube, unitPyramid, unitSphere, dummyState);
}

// ---------------------------------------------------------------------------
// 1. Foundation Plinth & Tapered Tower Shaft
// ---------------------------------------------------------------------------
void Windmill::drawBaseAndTower(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                                const glm::mat4& rootMat) const
{
    // Material Palettes
    const glm::vec3 stoneBase(0.32f, 0.30f, 0.35f);     // Chiseled foundation fieldstone
    const glm::vec3 stoneDark(0.22f, 0.20f, 0.25f);     // Weathered foundation mortar/rock
    const glm::vec3 timberLower(0.42f, 0.30f, 0.20f);   // Lower aged timber planks
    const glm::vec3 timberUpper(0.56f, 0.44f, 0.32f);   // Lighter upper clapboard cladding
    const glm::vec3 timberTrim(0.28f, 0.18f, 0.11f);    // Heavy dark timber framing & corbels
    const glm::vec3 ironMetal(0.18f, 0.18f, 0.20f);     // Door hinges & hardware
    const glm::vec3 doorWood(0.38f, 0.24f, 0.14f);      // Oak arched doorway
    const glm::vec3 windowGlow(1.0f, 0.74f, 0.30f);     // Warm glowing interior amber window light

    // --- A. TIERED STONE PLINTH ---
    // Ground foundation step
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.12f, 0.0f), glm::vec3(3.6f, 0.24f, 3.6f), stoneDark);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.12f, 0.0f), glm::vec3(3.5f, 0.24f, 3.5f), stoneDark, 45.0f);

    // Upper stone tier
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.34f, 0.0f), glm::vec3(3.1f, 0.22f, 3.1f), stoneBase);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.34f, 0.0f), glm::vec3(3.0f, 0.22f, 3.0f), stoneBase, 45.0f);

    // Front stone entrance steps
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.10f, 1.85f), glm::vec3(1.5f, 0.20f, 0.6f), stoneDark);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 0.22f, 1.60f), glm::vec3(1.3f, 0.22f, 0.5f), stoneBase);

    // --- B. TOWER TIER 1 (Lower Heavy Timber & Fieldstone Shaft, Y = 0.45 to 2.8m) ---
    // Octagonal tapered lower shaft
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 1.62f, 0.0f), glm::vec3(2.6f, 2.34f, 2.6f), timberLower);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 1.62f, 0.0f), glm::vec3(2.5f, 2.34f, 2.5f), timberLower, 45.0f);

    // Corner vertical timber reinforcement posts
    for (int i = 0; i < 4; ++i)
    {
        float angle = i * 90.0f + 45.0f;
        drawCube(shader, unitCube, rootMat,
                 glm::vec3(1.22f * std::cos(glm::radians(angle)), 1.62f, 1.22f * std::sin(glm::radians(angle))),
                 glm::vec3(0.18f, 2.34f, 0.18f), timberTrim, angle);
    }

    // Front Arched Entry Door (+Z face)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 1.15f, 1.34f), glm::vec3(0.96f, 1.60f, 0.12f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 1.12f, 1.38f), glm::vec3(0.82f, 1.45f, 0.08f), doorWood);
    // Iron strap hinges and door handle
    drawCube(shader, unitCube, rootMat, glm::vec3( 0.25f, 1.50f, 1.43f), glm::vec3(0.35f, 0.06f, 0.04f), ironMetal);
    drawCube(shader, unitCube, rootMat, glm::vec3( 0.25f, 0.85f, 1.43f), glm::vec3(0.35f, 0.06f, 0.04f), ironMetal);
    drawCube(shader, unitCube, rootMat, glm::vec3(-0.25f, 1.15f, 1.44f), glm::vec3(0.06f, 0.14f, 0.06f), ironMetal);

    // Horizontal timber transition ring 1 (Y = 2.8m)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 2.82f, 0.0f), glm::vec3(2.70f, 0.14f, 2.70f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 2.82f, 0.0f), glm::vec3(2.62f, 0.14f, 2.62f), timberTrim, 45.0f);

    // --- C. TOWER TIER 2 (Middle Weathered Cladding Shaft, Y = 2.8 to 5.2m) ---
    // Tapered middle shaft (narrower than base)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.02f, 0.0f), glm::vec3(2.20f, 2.30f, 2.20f), timberUpper);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.02f, 0.0f), glm::vec3(2.12f, 2.30f, 2.12f), timberUpper, 45.0f);

    // Corner posts
    for (int i = 0; i < 4; ++i)
    {
        float angle = i * 90.0f + 45.0f;
        drawCube(shader, unitCube, rootMat,
                 glm::vec3(1.04f * std::cos(glm::radians(angle)), 4.02f, 1.04f * std::sin(glm::radians(angle))),
                 glm::vec3(0.16f, 2.30f, 0.16f), timberTrim, angle);
    }

    // Cozy Glowing Windows on Middle Shaft
    // Front window (+Z)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.10f, 1.13f), glm::vec3(0.62f, 0.74f, 0.10f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.10f, 1.16f), glm::vec3(0.48f, 0.60f, 0.06f), windowGlow, 0.0f, 0.0f, 0.0f, true, windowGlow * 1.5f);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.10f, 1.19f), glm::vec3(0.06f, 0.58f, 0.04f), timberTrim); // Vertical mullion
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 4.10f, 1.19f), glm::vec3(0.46f, 0.06f, 0.04f), timberTrim); // Horizontal mullion
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 3.70f, 1.20f), glm::vec3(0.70f, 0.08f, 0.18f), timberTrim); // Window sill

    // Side windows (-X and +X)
    drawCube(shader, unitCube, rootMat, glm::vec3(-1.13f, 4.10f, 0.0f), glm::vec3(0.10f, 0.64f, 0.52f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(-1.16f, 4.10f, 0.0f), glm::vec3(0.06f, 0.52f, 0.40f), windowGlow, 0.0f, 0.0f, 0.0f, true, windowGlow * 1.3f);
    drawCube(shader, unitCube, rootMat, glm::vec3( 1.13f, 4.10f, 0.0f), glm::vec3(0.10f, 0.64f, 0.52f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3( 1.16f, 4.10f, 0.0f), glm::vec3(0.06f, 0.52f, 0.40f), windowGlow, 0.0f, 0.0f, 0.0f, true, windowGlow * 1.3f);

    // Horizontal timber transition ring 2 (Y = 5.2m)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 5.20f, 0.0f), glm::vec3(2.32f, 0.14f, 2.32f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 5.20f, 0.0f), glm::vec3(2.24f, 0.14f, 2.24f), timberTrim, 45.0f);

    // --- D. TOWER TIER 3 (Upper Shaft, Y = 5.2 to 6.7m) ---
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 5.96f, 0.0f), glm::vec3(1.82f, 1.48f, 1.82f), timberUpper);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 5.96f, 0.0f), glm::vec3(1.75f, 1.48f, 1.75f), timberUpper, 45.0f);

    // Upper front attic window (+Z)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 6.00f, 0.94f), glm::vec3(0.48f, 0.52f, 0.08f), timberTrim);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 6.00f, 0.96f), glm::vec3(0.36f, 0.40f, 0.06f), windowGlow, 0.0f, 0.0f, 0.0f, true, windowGlow * 1.4f);
}

// ---------------------------------------------------------------------------
// 2. Wraparound Observation Balcony & Rooftop Cap
// ---------------------------------------------------------------------------
void Windmill::drawBalconyAndCap(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                                 const Sphere& unitSphere, const glm::mat4& rootMat) const
{
    const glm::vec3 timberDeck(0.38f, 0.25f, 0.15f);    // Balcony plank floor
    const glm::vec3 timberRailing(0.26f, 0.17f, 0.10f); // Guard railings & corbels
    const glm::vec3 roofCapThatch(0.24f, 0.16f, 0.12f); // Dark rustic roof shingles
    const glm::vec3 roofRidge(0.34f, 0.22f, 0.16f);     // Roof ridge trim & rafters
    const glm::vec3 ironFinial(0.18f, 0.18f, 0.20f);    // Rooftop iron needle/weathervane

    // --- A. WRAPAROUND OBSERVATION BALCONY / GALLERY (Y = 6.70m) ---
    // Timber gallery floor
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 6.72f, 0.0f), glm::vec3(2.80f, 0.10f, 2.80f), timberDeck);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 6.72f, 0.0f), glm::vec3(2.70f, 0.10f, 2.70f), timberDeck, 45.0f);

    // 8 Diagonal wooden support corbels under the balcony deck
    for (int i = 0; i < 8; ++i)
    {
        float angle = i * 45.0f;
        float rad = glm::radians(angle);
        glm::vec3 pos(1.08f * std::cos(rad), 6.48f, 1.08f * std::sin(rad));
        drawCube(shader, unitCube, rootMat, pos, glm::vec3(0.12f, 0.36f, 0.44f), timberRailing, angle, 35.0f);
    }

    // Balcony perimeter posts & handrails
    const float railR = 1.34f;
    for (int i = 0; i < 8; ++i)
    {
        float angle = i * 45.0f;
        float rad = glm::radians(angle);
        // Post
        drawCube(shader, unitCube, rootMat,
                 glm::vec3(railR * std::cos(rad), 7.18f, railR * std::sin(rad)),
                 glm::vec3(0.08f, 0.82f, 0.08f), timberRailing);

        // Handrail segments linking adjacent posts
        float nextAngle = (i + 1) * 45.0f;
        float nextRad = glm::radians(nextAngle);
        glm::vec3 midP = 0.5f * (glm::vec3(railR * std::cos(rad), 7.55f, railR * std::sin(rad)) +
                                 glm::vec3(railR * std::cos(nextRad), 7.55f, railR * std::sin(nextRad)));
        drawCube(shader, unitCube, rootMat, midP, glm::vec3(1.08f, 0.06f, 0.08f), timberRailing, angle + 22.5f);
        // Mid-rail
        glm::vec3 midLow = 0.5f * (glm::vec3(railR * std::cos(rad), 7.15f, railR * std::sin(rad)) +
                                  glm::vec3(railR * std::cos(nextRad), 7.15f, railR * std::sin(nextRad)));
        drawCube(shader, unitCube, rootMat, midLow, glm::vec3(1.08f, 0.05f, 0.06f), timberRailing, angle + 22.5f);
    }

    // --- B. ROOFTOP ROTATING CAP (Y = 6.8 to 8.8m) ---
    // Cap Curb & Wall Housing
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.02f, 0.0f), glm::vec3(1.72f, 0.50f, 1.72f), timberDeck);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.02f, 0.0f), glm::vec3(1.65f, 0.50f, 1.65f), timberDeck, 45.0f);

    // Front Dormer / Rotor Axle Housing Hood (Projects forward at +Z)
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.42f, 0.85f), glm::vec3(0.92f, 0.72f, 1.15f), roofCapThatch);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.82f, 0.85f), glm::vec3(0.98f, 0.12f, 1.20f), roofRidge);

    // Pitched Conical / Pyramidal Roof Cap
    drawPyramid(shader, unitPyramid, rootMat, glm::vec3(0.0f, 7.78f, 0.0f), glm::vec3(2.15f, 1.85f, 2.15f), roofCapThatch);
    drawPyramid(shader, unitPyramid, rootMat, glm::vec3(0.0f, 7.78f, 0.0f), glm::vec3(2.05f, 1.85f, 2.05f), roofCapThatch, 45.0f);

    // Wooden Eaves / Cornice Trim
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.28f, 0.0f), glm::vec3(2.26f, 0.10f, 2.26f), roofRidge);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.28f, 0.0f), glm::vec3(2.20f, 0.10f, 2.20f), roofRidge, 45.0f);

    // Rooftop Weather Spire / Finial
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 9.25f, 0.0f), glm::vec3(0.08f, 1.10f, 0.08f), ironFinial);
    drawSphere(shader, unitSphere, rootMat, glm::vec3(0.0f, 9.75f, 0.0f), glm::vec3(0.12f), ironFinial);
    drawCube(shader, unitCube, rootMat, glm::vec3(0.20f, 9.60f, 0.0f), glm::vec3(0.48f, 0.06f, 0.02f), ironFinial);
    drawPyramid(shader, unitPyramid, rootMat, glm::vec3(0.48f, 9.60f, 0.0f), glm::vec3(0.12f, 0.18f, 0.02f), ironFinial, 0.0f, 0.0f, -90.0f);
}

// ---------------------------------------------------------------------------
// 3. Rotating Rotor Hub & 4 Aerodynamic Sail Blades
// ---------------------------------------------------------------------------
void Windmill::drawRotorAndSails(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid,
                                 const Sphere& unitSphere, const glm::mat4& rootMat, float rotationAngle) const
{
    // Materials
    const glm::vec3 sparWood(0.36f, 0.24f, 0.15f);      // Heavy pine rotor beams
    const glm::vec3 sparTrim(0.26f, 0.17f, 0.10f);      // Cross-lattice framework
    const glm::vec3 sailCanvas(0.92f, 0.88f, 0.82f);    // Cream canvas sailcloth
    const glm::vec3 ironHub(0.22f, 0.22f, 0.25f);       // Forged iron center hub & bands
    const glm::vec3 brassCone(0.85f, 0.70f, 0.28f);     // Brass hub nose cone

    // Hub Anchor Point: Front face of the cap at Y = 7.42m, Z = 1.38m
    const glm::vec3 hubCenter(0.0f, 7.42f, 1.38f);

    // Axle Shaft projecting from the cap
    drawCube(shader, unitCube, rootMat, glm::vec3(0.0f, 7.42f, 1.15f), glm::vec3(0.28f, 0.28f, 0.46f), ironHub);

    // Build the animated rotor matrix: Translated to hubCenter, rotated by rotationAngle around Z-axis
    glm::mat4 rotorMat = rootMat;
    rotorMat = glm::translate(rotorMat, hubCenter);
    // Continuous rotation around axle (local Z)
    rotorMat = glm::rotate(rotorMat, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    // Central Rotor Hub Block (Cylindrical / octagonal iron core)
    drawCube(shader, unitCube, rotorMat, glm::vec3(0.0f, 0.0f, 0.06f), glm::vec3(0.68f, 0.68f, 0.28f), ironHub);
    drawCube(shader, unitCube, rotorMat, glm::vec3(0.0f, 0.0f, 0.06f), glm::vec3(0.64f, 0.64f, 0.28f), ironHub, 45.0f);
    // Pointed hub nose cone pointing forward
    drawPyramid(shader, unitPyramid, rotorMat, glm::vec3(0.0f, 0.0f, 0.22f), glm::vec3(0.48f, 0.38f, 0.48f), brassCone, 0.0f, -90.0f, 0.0f);

    // --- 4 CROSS-SHAPED ROTOR BLADES / SAILS (At 0, 90, 180, 270 degrees) ---
    const float bladeLength = 4.30f; // Long majestic sails reaching 8.6m wingspan
    const float sparThickness = 0.12f;

    for (int k = 0; k < 4; ++k)
    {
        float bladeRotZ = k * 90.0f;
        glm::mat4 bladeMat = glm::rotate(rotorMat, glm::radians(bladeRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

        // 1. Main Structural Wooden Spar (Extends along +X in bladeMat)
        // Center of spar is at X = 0.35 + bladeLength / 2 = 2.50m
        drawCube(shader, unitCube, bladeMat,
                 glm::vec3(2.50f, 0.0f, 0.05f),
                 glm::vec3(bladeLength, sparThickness, sparThickness),
                 sparWood);

        // Iron hub mounting clamp
        drawCube(shader, unitCube, bladeMat,
                 glm::vec3(0.55f, 0.0f, 0.05f),
                 glm::vec3(0.36f, 0.18f, 0.18f),
                 ironHub);

        // 2. Transverse Lattice Framework Ribs (Crossbars perpendicular to spar)
        // Ribs project downward (-Y in bladeMat) to support the canvas
        const int ribCount = 6;
        for (int r = 0; r < ribCount; ++r)
        {
            float rx = 1.0f + r * 0.62f; // From 1.0m to 4.1m along the spar
            drawCube(shader, unitCube, bladeMat,
                     glm::vec3(rx, -0.36f, 0.05f),
                     glm::vec3(0.06f, 0.72f, 0.06f),
                     sparTrim);
        }

        // Outer Trailing Edge Spar
        drawCube(shader, unitCube, bladeMat,
                 glm::vec3(2.55f, -0.72f, 0.05f),
                 glm::vec3(3.20f, 0.06f, 0.06f),
                 sparTrim);

        // 3. Canvas Sailcloth Panel (Crisp cream fabric stretched on the lattice)
        // Spans along X from 1.0m to 4.1m (length 3.10m), along Y from 0.0 to -0.72m (width 0.68m)
        drawCube(shader, unitCube, bladeMat,
                 glm::vec3(2.55f, -0.36f, 0.06f),
                 glm::vec3(3.10f, 0.66f, 0.03f),
                 sailCanvas);

        // End decorative sail finial / tip
        drawPyramid(shader, unitPyramid, bladeMat,
                    glm::vec3(4.70f, 0.0f, 0.05f),
                    glm::vec3(0.14f, 0.22f, 0.14f),
                    sparWood, 0.0f, 0.0f, -90.0f);
    }
}
