#include "objects/Cabin.h"
#include "texture/TextureManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

Cabin::Cabin(const glm::vec3& pos, float rotY, float scale)
    : m_position(pos),
      m_rotationY(rotY),
      m_scaleFactor(scale),
      m_isDoorOpen(false),
      m_doorAngle(0.0f),
      m_targetDoorAngle(0.0f),
      m_isFireLit(false),
      m_woodPlacedCount(0),
      m_totalTime(0.0f),
      m_flameFlicker(1.0f)
{
}

void Cabin::setPosition(const glm::vec3& pos) { m_position = pos; }
void Cabin::setRotation(float rotY)           { m_rotationY = rotY; }
void Cabin::setScale(float scale)             { m_scaleFactor = scale; }

const glm::vec3& Cabin::getPosition() const   { return m_position; }
float Cabin::getRotation() const              { return m_rotationY; }
float Cabin::getScale() const                 { return m_scaleFactor; }

void Cabin::toggleDoor()
{
    setDoorOpen(!m_isDoorOpen);
}

void Cabin::setDoorOpen(bool open)
{
    m_isDoorOpen = open;
    m_targetDoorAngle = open ? -95.0f : 0.0f;
}

bool Cabin::isDoorOpen() const   { return m_isDoorOpen; }
float Cabin::getDoorAngle() const { return m_doorAngle; }

void Cabin::setFireLit(bool lit)
{
    m_isFireLit = lit;
    if (lit && m_woodPlacedCount == 0)
    {
        m_woodPlacedCount = 3; // Ensure logs are placed in the hearth
    }
}

void Cabin::toggleFire()
{
    setFireLit(!m_isFireLit);
}

bool Cabin::isFireLit() const     { return m_isFireLit; }
void Cabin::addWood()             { m_woodPlacedCount++; }
int Cabin::getWoodCount() const   { return m_woodPlacedCount; }

glm::mat4 Cabin::getRootMatrix() const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, m_position);
    root = glm::rotate(root, glm::radians(m_rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::scale(root, glm::vec3(m_scaleFactor));
    return root;
}

glm::vec3 Cabin::getDoorWorldPosition() const
{
    // Local door handle position is approx (-0.55, 1.20, 2.45)
    glm::mat4 root = getRootMatrix();
    glm::vec4 worldPos = root * glm::vec4(-0.55f, 1.20f, 2.45f, 1.0f);
    return glm::vec3(worldPos);
}

glm::vec3 Cabin::getFireplaceWorldPosition() const
{
    // Local hearth center is approx (2.05, 0.75, -0.60)
    glm::mat4 root = getRootMatrix();
    glm::vec4 worldPos = root * glm::vec4(2.05f, 0.75f, -0.60f, 1.0f);
    return glm::vec3(worldPos);
}

glm::vec3 Cabin::getChimneyWorldPosition() const
{
    // Local chimney flue top is approx (2.05, 5.75, -0.60)
    glm::mat4 root = getRootMatrix();
    glm::vec4 worldPos = root * glm::vec4(2.05f, 5.75f, -0.60f, 1.0f);
    return glm::vec3(worldPos);
}

glm::vec3 Cabin::getFireplaceLightPosition() const
{
    return getFireplaceWorldPosition() + glm::vec3(0.0f, 0.25f, 0.0f);
}

glm::vec3 Cabin::getFireplaceLightColor(float time) const
{
    if (!m_isFireLit) return glm::vec3(0.0f);
    // Warm incandescent campfire & hearth amber-orange glow
    float r = 1.0f;
    float g = 0.55f + 0.06f * std::sin(time * 11.0f);
    float b = 0.15f + 0.04f * std::cos(time * 19.0f);
    return glm::vec3(r, g, b);
}

float Cabin::getFireplaceLightIntensity(float time) const
{
    if (!m_isFireLit) return 0.0f;
    // Radiant flickering hearth light
    return 2.4f + 0.45f * std::sin(time * 13.0f) * std::cos(time * 21.0f);
}

void Cabin::update(float dt, float totalTime)
{
    m_totalTime = totalTime;

    // Smooth exponential ease towards target door open angle
    m_doorAngle = glm::mix(m_doorAngle, m_targetDoorAngle, 1.0f - std::exp(-8.0f * dt));
    if (std::abs(m_doorAngle - m_targetDoorAngle) < 0.15f)
    {
        m_doorAngle = m_targetDoorAngle;
    }

    // Dynamic fire turbulence harmonics
    m_flameFlicker = 1.0f + 0.18f * std::sin(18.0f * totalTime) + 0.12f * std::cos(27.0f * totalTime);
}

// ---------------------------------------------------------------------------
// Unit Primitive Draw Helpers
// ---------------------------------------------------------------------------
void Cabin::drawCubePart(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                         const glm::vec3& localOffset, const glm::vec3& localScale,
                         const glm::vec3& color,
                         float rotY, float rotX, float rotZ,
                         bool isEmissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", isEmissive);
    if (isEmissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitCube.draw();
}

void Cabin::drawTriangularPrismPart(const Shader& shader, const TriangularPrism& unitPrism, const glm::mat4& parentMat,
                                    const glm::vec3& localOffset, const glm::vec3& localScale,
                                    const glm::vec3& color,
                                    float rotY, float rotX, float rotZ) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", false);

    unitPrism.draw();
}

void Cabin::drawCylinderPart(const Shader& shader, const Cylinder& unitCylinder, const glm::mat4& parentMat,
                              const glm::vec3& localOffset, const glm::vec3& localScale,
                              const glm::vec3& color,
                              float rotY, float rotX, float rotZ) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", false);

    unitCylinder.draw();
}

void Cabin::drawSpherePart(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                           const glm::vec3& localOffset, const glm::vec3& localScale,
                           const glm::vec3& color,
                           bool isEmissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", isEmissive);
    if (isEmissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitSphere.draw();
}

void Cabin::drawPyramidPart(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
                            const glm::vec3& localOffset, const glm::vec3& localScale,
                            const glm::vec3& color,
                            float rotY, float rotX, float rotZ,
                            bool isEmissive, const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", isEmissive);
    if (isEmissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitPyramid.draw();
}

// ---------------------------------------------------------------------------
// Main Hierarchical Draw Passes
// ---------------------------------------------------------------------------
void Cabin::draw(const Shader& shader,
                 const Cube& unitCube,
                 const TriangularPrism& unitPrism,
                 const Cylinder& unitCylinder,
                 const Sphere& unitSphere,
                 const Pyramid& unitPyramid) const
{
    glm::mat4 rootMat = getRootMatrix();

    // 1. Exterior Architecture (Foundation, floor, walls with doorway opening, roof, porch)
    TextureManager::getInstance().setTextureMode(shader, TEX_WOOD, 0.85f);
    drawExterior(shader, unitCube, unitPrism, unitCylinder, rootMat);

    // 2. Interactive Front Door (Hinged swinging panel, brass handle)
    TextureManager::getInstance().setTextureMode(shader, TEX_WOOD, 0.90f);
    drawInteractiveDoor(shader, unitCube, rootMat);

    // 3. Realistic Stone Fireplace (Fieldstone masonry, mantel, ornaments, firebox grate, crackling fire)
    drawFireplace(shader, unitCube, unitCylinder, unitPyramid, unitSphere, rootMat);

    // 4. Procedural Chimney Smoke (Wispy, soft, atmospheric plume with alpha blending)
    drawChimneySmoke(shader, unitSphere, rootMat);

    // 5. Interior Furniture (Bed, table, chairs, bookshelf, rug - ZERO wood texture or wood color)
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);
    drawInteriorFurniture(shader, unitCube, unitCylinder, unitSphere, rootMat);

    // 6. Architectural Semi-Transparent Glass Windows (Front, Lake-side, and Attic)
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);
    drawWindowsGlass(shader, unitCube, rootMat);

    // Reset shader state & texture mode
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);
    shader.setBool("isEmissive", false);
    shader.setBool("isGlass", false);
    shader.setFloat("materialAlpha", 1.0f);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}


// ---------------------------------------------------------------------------
// 1. Exterior Architecture & Building Envelope
// ---------------------------------------------------------------------------
void Cabin::drawExterior(const Shader& shader,
                         const Cube& unitCube,
                         const TriangularPrism& unitPrism,
                         const Cylinder& unitCylinder,
                         const glm::mat4& rootMat) const
{
    // Color Palette
    const glm::vec3 colStoneFoundation(0.25f, 0.23f, 0.21f); // Weathered granite foundation
    const glm::vec3 colTimberWall(0.46f, 0.28f, 0.17f);      // Warm rustic cedar logs
    const glm::vec3 colTimberDark(0.26f, 0.15f, 0.09f);      // Structural corner posts, lintels & trim
    const glm::vec3 colFloorPlanks(0.38f, 0.24f, 0.15f);     // Hardwood oak timber floorboards
    const glm::vec3 colRoofShingles(0.21f, 0.19f, 0.18f);    // Weathered alpine slate roof shingles
    const glm::vec3 colWindowGlow(1.0f, 0.82f, 0.38f);      // Warm amber lantern glow
    const glm::vec3 colChimneyStone(0.33f, 0.33f, 0.36f);   // Chimney masonry stone
    const glm::vec3 colChimneyFlue(0.44f, 0.26f, 0.19f);    // Terracotta clay chimney flue pot

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 10.0f);

    // A. FOUNDATION & FLOORING
    // Stone foundation slab slightly raised above meadow ground
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 0.16f, 0.0f), glm::vec3(5.60f, 0.32f, 5.20f), colStoneFoundation);

    // Polished interior timber floorboards
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 0.34f, 0.0f), glm::vec3(5.40f, 0.06f, 5.00f), colFloorPlanks);

    // B. LOG WALLS WITH OPEN DOORWAY OPENING (Wall height 2.85m, Y spanning [0.35, 3.20])
    const float wallH = 2.85f;
    const float wallCenterY = 0.35f + wallH * 0.5f; // 1.775m

    // Left Wall (-X)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-2.61f, wallCenterY, 0.0f), glm::vec3(0.20f, wallH, 4.80f), colTimberWall);

    // Right Wall (+X, Facing Lake) - Framed with physical window opening
    // Window opening centered at Z = 0.60, Y = 1.65, width in Z = 1.10, height in Y = 1.15
    // Section A: Back wall (Z from -2.40 to 0.05, depth 2.45m, center Z = -1.175m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, wallCenterY, -1.175f), glm::vec3(0.20f, wallH, 2.45f), colTimberWall);
    // Section B: Front wall (Z from 1.15 to 2.40, depth 1.25m, center Z = 1.775m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, wallCenterY,  1.775f), glm::vec3(0.20f, wallH, 1.25f), colTimberWall);
    // Section C: Wall under window (Z from 0.05 to 1.15, depth 1.10m, center Z = 0.60m, Y in [0.35, 1.075], height 0.725m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 0.7125f, 0.60f), glm::vec3(0.20f, 0.725f, 1.10f), colTimberWall);
    // Section D: Wall header above window (Z from 0.05 to 1.15, depth 1.10m, center Z = 0.60m, Y in [2.225, 3.20], height 0.975m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 2.7125f, 0.60f), glm::vec3(0.20f, 0.975f, 1.10f), colTimberWall);

    // Back Wall (-Z)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, wallCenterY, -2.41f), glm::vec3(5.02f, wallH, 0.20f), colTimberWall);

    // Front Wall (+Z): Carefully split to leave an open hollow doorway opening and open window opening!
    // Doorway spans X from -1.15m to +0.05m, height from Y = 0.35m to 2.55m
    // Left front wall section (from left corner X = -2.60 to door jamb X = -1.15)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.875f, wallCenterY, 2.41f), glm::vec3(1.45f, wallH, 0.20f), colTimberWall);

    // Right front wall section - Framed with physical window opening (X = 1.30, Y = 1.65)
    // Section A: Left pillar between door jamb & window (X from 0.05 to 0.75, width 0.70m, center X = 0.40m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.40f, wallCenterY, 2.41f), glm::vec3(0.70f, wallH, 0.20f), colTimberWall);
    // Section B: Right pillar between window & corner post (X from 1.85 to 2.60, width 0.75m, center X = 2.225m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.225f, wallCenterY, 2.41f), glm::vec3(0.75f, wallH, 0.20f), colTimberWall);
    // Section C: Wall apron under window (X from 0.75 to 1.85, width 1.10m, center X = 1.30m, Y in [0.35, 1.075], height 0.725m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 0.7125f, 2.41f), glm::vec3(1.10f, 0.725f, 0.20f), colTimberWall);
    // Section D: Wall header above window (X from 0.75 to 1.85, width 1.10m, center X = 1.30m, Y in [2.225, 3.20], height 0.975m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 2.7125f, 2.41f), glm::vec3(1.10f, 0.975f, 0.20f), colTimberWall);


    // Doorway Header Lintel Beam (above door, spanning Y = [2.55, 3.20])
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.55f, 2.875f, 2.41f), glm::vec3(1.20f, 0.65f, 0.20f), colTimberWall);

    // 4 Structural Corner Timber Posts
    const float postW = 0.28f;
    const float postH = 2.92f;
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-2.62f, wallCenterY, -2.42f), glm::vec3(postW, postH, postW), colTimberDark);
    drawCubePart(shader, unitCube, rootMat, glm::vec3( 2.62f, wallCenterY, -2.42f), glm::vec3(postW, postH, postW), colTimberDark);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-2.62f, wallCenterY,  2.42f), glm::vec3(postW, postH, postW), colTimberDark);
    drawCubePart(shader, unitCube, rootMat, glm::vec3( 2.62f, wallCenterY,  2.42f), glm::vec3(postW, postH, postW), colTimberDark);

    // Front Header Trim Beam across the top of the front wall
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 3.15f, 2.48f), glm::vec3(5.45f, 0.16f, 0.16f), colTimberDark);

    // C. ARCHITECTURAL WINDOW CASING FRAMES, SILLS & CROSS MUNTINS
    // 1. Front Window Casement Frame & Sills (X = 1.30, Y = 1.65, Z = 2.41)
    // Left jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.75f, 1.65f, 2.41f), glm::vec3(0.08f, 1.15f, 0.22f), colTimberDark);
    // Right jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.85f, 1.65f, 2.41f), glm::vec3(0.08f, 1.15f, 0.22f), colTimberDark);
    // Top header
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 2.225f, 2.41f), glm::vec3(1.18f, 0.08f, 0.22f), colTimberDark);
    // Protruding bottom window sill
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 1.04f, 2.43f), glm::vec3(1.24f, 0.08f, 0.26f), colTimberDark);
    // Cross Muntin Bars (4-pane window grid)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 1.65f, 2.41f), glm::vec3(0.04f, 1.10f, 0.08f), colTimberDark);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 1.65f, 2.41f), glm::vec3(1.02f, 0.04f, 0.08f), colTimberDark);

    // 2. Lake-Facing Side Window Casement Frame & Sills (X = 2.61, Y = 1.65, Z = 0.60)
    // Back jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 1.65f, 0.05f), glm::vec3(0.22f, 1.15f, 0.08f), colTimberDark);
    // Front jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 1.65f, 1.15f), glm::vec3(0.22f, 1.15f, 0.08f), colTimberDark);
    // Top header
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 2.225f, 0.60f), glm::vec3(0.22f, 0.08f, 1.18f), colTimberDark);
    // Protruding bottom window sill
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.63f, 1.04f, 0.60f), glm::vec3(0.26f, 0.08f, 1.24f), colTimberDark);
    // Cross Muntin Bars (4-pane window grid)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 1.65f, 0.60f), glm::vec3(0.08f, 1.10f, 0.04f), colTimberDark);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 1.65f, 0.60f), glm::vec3(0.08f, 0.04f, 1.02f), colTimberDark);

    // D. HIGH VAULTED GABLED ROOF (TriangularPrism)
    // Sits atop walls at Y = 3.20m, spanning width 6.20m, height 1.95m, depth 5.80m (Apex at Y = 5.15m)
    TextureManager::getInstance().setTextureMode(shader, TEX_ROOF, 1.2f);
    drawTriangularPrismPart(shader, unitPrism, rootMat,
                            glm::vec3(0.0f, 3.20f, 0.0f), glm::vec3(6.20f, 1.95f, 5.80f),
                            colRoofShingles);
    TextureManager::getInstance().setTextureMode(shader, TEX_WOOD, 0.85f);

    // Roof Ridge Cap Beam
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 5.16f, 0.0f), glm::vec3(0.24f, 0.16f, 5.85f), colTimberDark);

    // Attic Gable Window on front triangular face (Frame & Muntins)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 3.85f, 2.82f), glm::vec3(0.72f, 0.64f, 0.08f), colTimberDark);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 3.85f, 2.84f), glm::vec3(0.03f, 0.54f, 0.04f), colTimberDark);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 3.85f, 2.84f), glm::vec3(0.60f, 0.03f, 0.04f), colTimberDark);


    // E. EXTENDED FRONT PORCH DECK, CANOPY & STEPS
    // Porch Deck Platform
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 0.22f, 3.40f), glm::vec3(5.40f, 0.20f, 2.00f), colFloorPlanks);

    // Front Wooden Steps
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 0.11f, 4.55f), glm::vec3(2.40f, 0.14f, 0.50f), colFloorPlanks);

    // 3 Turned Timber Porch Columns supporting canopy
    const float porchPostX[3] = { -2.45f, 0.0f, 2.45f };
    for (int i = 0; i < 3; ++i)
    {
        drawCubePart(shader, unitCube, rootMat,
                     glm::vec3(porchPostX[i], 1.60f, 4.25f), glm::vec3(0.18f, 2.65f, 0.18f), colTimberDark);
    }

    // Porch Canopy Roof Overhang
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 2.95f, 3.45f), glm::vec3(5.60f, 0.16f, 2.15f), colRoofShingles);
}

// ---------------------------------------------------------------------------
// 2. Interactive Front Door (Openable / Closable on Hinge)
// ---------------------------------------------------------------------------
void Cabin::drawInteractiveDoor(const Shader& shader,
                                const Cube& unitCube,
                                const glm::mat4& rootMat) const
{
    const glm::vec3 colTimberDark(0.26f, 0.15f, 0.09f);
    const glm::vec3 colDoorPlanks(0.42f, 0.26f, 0.15f);
    const glm::vec3 colIronHardware(0.12f, 0.12f, 0.12f);
    const glm::vec3 colBrassKnob(0.88f, 0.76f, 0.22f);

    // Door Jambs & Frame (fixed to doorway opening)
    // Left Jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.20f, 1.45f, 2.44f), glm::vec3(0.10f, 2.25f, 0.12f), colTimberDark);
    // Right Jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3( 0.05f, 1.45f, 2.44f), glm::vec3(0.10f, 2.25f, 0.12f), colTimberDark);
    // Top Lintel Frame
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.575f, 2.58f, 2.44f), glm::vec3(1.35f, 0.12f, 0.12f), colTimberDark);

    // Hinged Door Assembly
    // Hinge anchor point is at the left jamb: X = -1.15m, Z = 2.44m
    glm::mat4 doorMat = rootMat;
    doorMat = glm::translate(doorMat, glm::vec3(-1.15f, 0.35f, 2.44f));
    doorMat = glm::rotate(doorMat, glm::radians(m_doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Door Panel geometry (Offset from hinge by half width +0.575m, Y center +1.10m)
    // Main Solid Timber Door Panel (Width 1.15m, Height 2.20m, Depth 0.08m)
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(0.575f, 1.10f, 0.0f), glm::vec3(1.15f, 2.20f, 0.08f), colDoorPlanks);

    // Recessed Vertical Cedar Inset Planks
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(0.575f, 1.10f, 0.02f), glm::vec3(1.00f, 2.05f, 0.04f), colDoorPlanks * 0.90f);

    // Decorative Black Iron Strap Hinges (Top & Bottom horizontal bands)
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(0.40f, 1.85f, 0.05f), glm::vec3(0.70f, 0.06f, 0.03f), colIronHardware);
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(0.40f, 0.35f, 0.05f), glm::vec3(0.70f, 0.06f, 0.03f), colIronHardware);

    // Polished Brass Doorknob & Keyplate (near open edge: X = +1.02m)
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(1.02f, 1.05f, 0.06f), glm::vec3(0.06f, 0.14f, 0.02f), colIronHardware);
    drawCubePart(shader, unitCube, doorMat,
                 glm::vec3(1.02f, 1.05f, 0.08f), glm::vec3(0.08f, 0.08f, 0.08f), colBrassKnob);
}

// ---------------------------------------------------------------------------
// 3. Realistic Rustic Stone & Dark Brick Fireplace, Hearth & Chimney
// ---------------------------------------------------------------------------
void Cabin::drawFireplace(const Shader& shader,
                          const Cube& unitCube,
                          const Cylinder& unitCylinder,
                          const Pyramid& unitPyramid,
                          const Sphere& unitSphere,
                          const glm::mat4& rootMat) const
{
    // Fireplace is built on the right wall (+X): Local X = 2.05m, Z = -0.60m, facing inward into the room (-X)
    // Realistic masonry palette: natural rustic stone, dark soot-stained cavity, cast iron, and hewn slate
    const glm::vec3 colStoneMain(0.38f, 0.36f, 0.35f);       // Weathered granite/fieldstone masonry
    const glm::vec3 colStoneDark(0.28f, 0.26f, 0.25f);       // Deep shadow masonry blocks
    const glm::vec3 colStoneWarm(0.44f, 0.40f, 0.36f);       // Earthy sandstone accents
    const glm::vec3 colStoneLintel(0.25f, 0.24f, 0.24f);     // Hand-hewn dark slate arch & lintel
    const glm::vec3 colSlateMantel(0.22f, 0.21f, 0.22f);     // Heavy rustic slate mantelpiece
    const glm::vec3 colHearthTile(0.26f, 0.25f, 0.26f);      // Dark slate floor hearth apron
    const glm::vec3 colHearthCurb(0.20f, 0.19f, 0.20f);      // Beveled slate curb edging
    const glm::vec3 colFireboxCavity(0.07f, 0.06f, 0.06f);   // Matte soot-blackened firebox interior
    const glm::vec3 colCastIron(0.14f, 0.14f, 0.15f);        // Wrought iron grate, andirons & tools
    const glm::vec3 colBrass(0.82f, 0.70f, 0.24f);           // Vintage brass mantel clock & candlesticks
    const glm::vec3 colCharLog(0.16f, 0.12f, 0.09f);         // Charred timber logs
    const glm::vec3 colBirchBark(0.85f, 0.82f, 0.76f);       // Fresh birch firewood in hearth cradle

    // Set cobblestone texture for the exterior stone masonry
    TextureManager::getInstance().setTextureMode(shader, TEX_COBBLESTONE, 1.4f);

    // A. Main Fieldstone Chimney Breast Body (from floor Y = 0.35m to Y = 2.15m)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.15f, 1.25f, -0.60f), glm::vec3(0.95f, 1.80f, 1.95f), colStoneMain);

    // Natural stone interlocking blocks along the sides
    const float sideZ[4] = { -1.50f, -1.40f, 0.20f, 0.30f };
    drawCubePart(shader, unitCube, rootMat, glm::vec3(2.05f, 0.75f, sideZ[0]), glm::vec3(0.22f, 0.38f, 0.22f), colStoneDark);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(2.05f, 1.35f, sideZ[1]), glm::vec3(0.24f, 0.36f, 0.24f), colStoneWarm);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(2.05f, 0.75f, sideZ[2]), glm::vec3(0.22f, 0.38f, 0.22f), colStoneWarm);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(2.05f, 1.35f, sideZ[3]), glm::vec3(0.24f, 0.36f, 0.24f), colStoneDark);

    // Tapered stone smoke chamber breast above mantel (Y = 2.15m to Y = 3.20m ceiling)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.18f, 2.68f, -0.60f), glm::vec3(0.85f, 1.05f, 1.40f), colStoneMain);

    // Upper Chimney Shaft extending through ceiling and roof to the sky
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.15f, 4.35f, -0.60f), glm::vec3(0.85f, 2.30f, 0.90f), colStoneDark);
    // Heavy slate chimney capping stone
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.15f, 5.50f, -0.60f), glm::vec3(1.02f, 0.16f, 1.02f), colStoneLintel);
    // Terracotta chimney flue pot
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(2.15f, 5.68f, -0.60f), glm::vec3(0.42f, 0.40f, 0.42f), glm::vec3(0.55f, 0.28f, 0.18f));

    // Turn off texture for refined architectural details
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);

    // B. Recessed Soot-Blackened Firebox Cavity
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.05f, 1.05f, -0.60f), glm::vec3(0.75f, 1.25f, 1.15f), colFireboxCavity);

    // C. Heavy Stone Jambs & Structural Lintel framing the opening
    // Left stone jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.64f, 1.05f, -1.22f), glm::vec3(0.24f, 1.25f, 0.22f), colStoneLintel);
    // Right stone jamb
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.64f, 1.05f,  0.02f), glm::vec3(0.24f, 1.25f, 0.22f), colStoneLintel);
    // Robust stone lintel beam spanning the top of the opening
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.64f, 1.70f, -0.60f), glm::vec3(0.26f, 0.22f, 1.30f), colStoneLintel);
    // Keystone in the center of the lintel
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.62f, 1.74f, -0.60f), glm::vec3(0.28f, 0.26f, 0.26f), colStoneMain);

    // D. Solid Hand-Hewn Slate Stone / Timber Mantelshelf
    // Main mantel shelf beam
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.58f, 2.15f, -0.60f), glm::vec3(0.42f, 0.16f, 2.15f), colSlateMantel);
    // 2 Sculpted supporting stone corbels
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.64f, 1.98f, -1.45f), glm::vec3(0.28f, 0.20f, 0.18f), colStoneLintel);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.64f, 1.98f,  0.25f), glm::vec3(0.28f, 0.20f, 0.18f), colStoneLintel);

    // Realistic Mantelpiece Ornaments:
    // Antique brass mantel clock in center
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.56f, 2.32f, -0.60f), glm::vec3(0.12f, 0.18f, 0.22f), colBrass);
    // 2 Elegant brass candlesticks on the ends
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(1.56f, 2.30f, -1.30f), glm::vec3(0.06f, 0.14f, 0.06f), colBrass);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(1.56f, 2.30f,  0.10f), glm::vec3(0.06f, 0.14f, 0.06f), colBrass);

    // E. Raised Hearth Floor Platform & Front Apron
    // Main raised slate stone floor slab
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 0.38f, -0.60f), glm::vec3(0.85f, 0.06f, 1.95f), colHearthTile);
    // Beveled stone curb front edge
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.85f, 0.38f, -0.60f), glm::vec3(0.08f, 0.08f, 1.95f), colHearthCurb);

    // F. Authentic Cast-Iron Fire Grate & Andirons (TEX_METAL)
    TextureManager::getInstance().setTextureMode(shader, TEX_METAL, 1.0f);
    // Horizontal grate bars elevating firewood off hearth floor
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.82f, 0.44f, -0.60f), glm::vec3(0.38f, 0.04f, 0.70f), colCastIron);
    // Left andiron upright post and finial
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.68f, 0.52f, -0.92f), glm::vec3(0.06f, 0.24f, 0.06f), colCastIron);
    drawSpherePart(shader, unitSphere, rootMat,
                   glm::vec3(1.68f, 0.66f, -0.92f), glm::vec3(0.08f, 0.08f, 0.08f), colCastIron);
    // Right andiron upright post and finial
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.68f, 0.52f, -0.28f), glm::vec3(0.06f, 0.24f, 0.06f), colCastIron);
    drawSpherePart(shader, unitSphere, rootMat,
                   glm::vec3(1.68f, 0.66f, -0.28f), glm::vec3(0.08f, 0.08f, 0.08f), colCastIron);

    // Wrought-iron fireplace tool stand beside hearth
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.15f, 0.42f, 0.45f), glm::vec3(0.18f, 0.04f, 0.18f), colCastIron);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(1.15f, 0.78f, 0.45f), glm::vec3(0.03f, 0.72f, 0.03f), colCastIron);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.18f, 0.74f, 0.48f), glm::vec3(0.03f, 0.68f, 0.03f), colCastIron, 12.0f);

    // Cast-iron firewood cradle on left side of hearth with birch logs
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.18f, 0.44f, -1.55f), glm::vec3(0.24f, 0.12f, 0.48f), colCastIron);
    TextureManager::getInstance().setTextureMode(shader, TEX_BARK, 0.8f);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(1.18f, 0.52f, -1.55f), glm::vec3(0.10f, 0.42f, 0.10f), colBirchBark, 0.0f, 0.0f, 90.0f);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(1.18f, 0.60f, -1.55f), glm::vec3(0.09f, 0.40f, 0.09f), colBirchBark, 0.0f, 0.0f, 90.0f);

    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);

    // G. Stacked Burning Firewood Logs on the Grate
    if (m_woodPlacedCount > 0 || m_isFireLit)
    {
        // Bottom front log
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(1.80f, 0.48f, -0.60f), glm::vec3(0.14f, 0.65f, 0.14f),
                         colCharLog, 0.0f, 0.0f, 90.0f);
        // Rear support log
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(1.92f, 0.50f, -0.60f), glm::vec3(0.13f, 0.62f, 0.13f),
                         colCharLog, 0.0f, 0.0f, 90.0f);
        // Crossed leaning log
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(1.85f, 0.56f, -0.58f), glm::vec3(0.12f, 0.58f, 0.12f),
                         colCharLog, 24.0f, 0.0f, 85.0f);
    }

    // H. Realistic Fire & Glowing Embers inside the Firebox
    if (m_isFireLit)
    {
        float flameH = 1.0f * m_flameFlicker;

        // 1. Incandescent Glowing Ember Bed beneath the grate
        glm::vec3 emberEmit = glm::vec3(1.0f, 0.36f, 0.05f) * (2.2f + 0.3f * std::sin(m_totalTime * 12.0f));
        drawSpherePart(shader, unitSphere, rootMat,
                       glm::vec3(1.84f, 0.44f, -0.60f), glm::vec3(0.32f, 0.10f, 0.48f),
                       glm::vec3(1.0f, 0.35f, 0.05f), true, emberEmit);

        // 2. Realistic Dynamic Flame Tongues (Kept inside firebox cavity!)
        const glm::vec3 colOuterFlame(0.95f, 0.22f, 0.04f);
        const glm::vec3 colMidFlame(1.0f, 0.52f, 0.08f);
        const glm::vec3 colInnerFlame(1.0f, 0.92f, 0.45f);

        // Center primary flame dancing up toward smoke chamber
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.84f, 0.56f, -0.60f), glm::vec3(0.28f, 0.72f * flameH, 0.38f),
                        colOuterFlame, 0.0f, 0.0f, 0.0f, true, colOuterFlame * 2.5f);
        // Left licking flame tongue
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.86f, 0.56f, -0.74f), glm::vec3(0.18f, 0.56f * flameH, 0.20f),
                        colOuterFlame, 10.0f, 0.0f, 0.0f, true, colOuterFlame * 2.2f);
        // Right licking flame tongue
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.82f, 0.56f, -0.46f), glm::vec3(0.19f, 0.60f * flameH, 0.22f),
                        colOuterFlame, -12.0f, 0.0f, 0.0f, true, colOuterFlame * 2.3f);

        // Mid vibrant fiery orange spires
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.82f, 0.60f, -0.60f), glm::vec3(0.22f, 0.62f * flameH, 0.28f),
                        colMidFlame, 0.0f, 0.0f, 0.0f, true, colMidFlame * 3.0f);
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.83f, 0.60f, -0.68f), glm::vec3(0.15f, 0.48f * flameH, 0.16f),
                        colMidFlame, 6.0f, 0.0f, 0.0f, true, colMidFlame * 2.6f);
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.81f, 0.60f, -0.52f), glm::vec3(0.16f, 0.52f * flameH, 0.18f),
                        colMidFlame, -8.0f, 0.0f, 0.0f, true, colMidFlame * 2.7f);

        // Core incandescent golden heart
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(1.80f, 0.64f, -0.60f), glm::vec3(0.14f, 0.48f * flameH, 0.18f),
                        colInnerFlame, 0.0f, 0.0f, 0.0f, true, colInnerFlame * 3.8f);

        // White-hot combustion heart
        drawSpherePart(shader, unitSphere, rootMat,
                       glm::vec3(1.80f, 0.58f, -0.60f), glm::vec3(0.10f, 0.16f, 0.12f),
                       glm::vec3(1.0f, 1.0f, 0.88f), true, glm::vec3(1.0f, 1.0f, 0.90f) * 4.5f);
    }
}

// ---------------------------------------------------------------------------
// 4. Procedural Billowing Chimney Smoke (Wispy, Soft, Atmospheric Plume)
// ---------------------------------------------------------------------------
void Cabin::drawChimneySmoke(const Shader& shader,
                             const Sphere& unitSphere,
                             const glm::mat4& rootMat) const
{
    if (!m_isFireLit) return;

    // Chimney flue pot top is at (2.15, 5.75, -0.60) in local cabin space
    const glm::vec3 chimneyTop(2.15f, 5.75f, -0.60f);

    // Disable textures and specular highlights for soft atmospheric smoke
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);
    shader.setFloat("shininess", 1.0f);
    shader.setBool("isEmissive", false);

    // Enable alpha blending and disable depth mask to prevent bubble silhouettes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    const int numPuffs = 36;
    const float maxRise = 8.5f;

    for (int i = 0; i < numPuffs; ++i)
    {
        // Continuous staggered rise progression based on time
        float progress = std::fmod((float)i * (maxRise / (float)numPuffs) + m_totalTime * 1.5f, maxRise);
        float h = progress / maxRise; // Normalized height [0.0, 1.0]

        // Vertical elevation
        float yOffset = progress;

        // Realistic atmospheric wind shear and turbulent curl sway
        float windX = 0.55f * h * h + 0.20f * std::sin(m_totalTime * 1.8f + (float)i * 0.7f) * h;
        float windZ = -0.40f * h * h + 0.18f * std::cos(m_totalTime * 1.4f + (float)i * 0.85f) * h;

        // Wispy elongated scale: stretched vertically along updraft, expanded radially
        float scaleX = 0.22f + 0.45f * h + 0.06f * std::sin(m_totalTime * 2.5f + (float)i);
        float scaleY = 0.38f + 0.70f * h; // Stretched vertically into plumes (NOT round spheres!)
        float scaleZ = 0.22f + 0.45f * h + 0.06f * std::cos(m_totalTime * 2.2f + (float)i);

        // Gradual atmospheric alpha dissipation: starts denser at rim, fades into cold air
        float alpha = (1.0f - h) * (1.0f - h) * 0.32f;
        shader.setFloat("materialAlpha", alpha);

        // Ash-grey / mist tone slightly diffusing into sky
        glm::vec3 puffColor = glm::mix(glm::vec3(0.68f, 0.69f, 0.72f), glm::vec3(0.50f, 0.52f, 0.56f), h);

        glm::vec3 puffPos = chimneyTop + glm::vec3(windX, yOffset, windZ);

        drawSpherePart(shader, unitSphere, rootMat, puffPos, glm::vec3(scaleX, scaleY, scaleZ), puffColor);
    }

    // Restore standard rendering state
    shader.setFloat("materialAlpha", 1.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// 5. Cabin Interior Furniture & Cozy Decor (Zero Wood-Like Texture or Color)
// ---------------------------------------------------------------------------
void Cabin::drawInteriorFurniture(const Shader& shader,
                                  const Cube& unitCube,
                                  const Cylinder& unitCylinder,
                                  const Sphere& unitSphere,
                                  const glm::mat4& rootMat) const
{
    // Explicitly disable any texture mode so furniture has zero wood texture
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE, 1.0f);

    // Elegant non-wood palette: matte wrought-iron, crisp white linen, warm crimson quilt,
    // polished stone/cream surfaces, and brass accents
    const glm::vec3 colIronMetal(0.18f, 0.18f, 0.20f);     // Matte black wrought-iron frame
    const glm::vec3 colBrass(0.85f, 0.74f, 0.26f);         // Polished vintage brass finials & accents
    const glm::vec3 colMattress(0.96f, 0.96f, 0.97f);      // Crisp clean white upholstery
    const glm::vec3 colDuvet(0.55f, 0.14f, 0.16f);         // Rich cozy deep crimson wool duvet
    const glm::vec3 colRunner(0.92f, 0.90f, 0.85f);        // Cream linen folded bed runner
    const glm::vec3 colPillow(0.97f, 0.97f, 0.96f);        // Plush soft white down pillows
    const glm::vec3 colPaintedCream(0.88f, 0.86f, 0.83f);  // Clean painted cream nightstand / table
    const glm::vec3 colChairCushion(0.84f, 0.82f, 0.78f);  // Oatmeal fabric chair cushions
    const glm::vec3 colRugWine(0.48f, 0.16f, 0.14f);       // Burgundy woven tapestry wool rug
    const glm::vec3 colRugBorder(0.90f, 0.88f, 0.80f);     // Cream fringe rug border
    const glm::vec3 colLanternGlow(1.0f, 0.82f, 0.35f);    // Warm amber lantern glow

    // =======================================================================
    // A. WROUGHT-IRON & LINEN VINTAGE BED (Rear-Left Bedroom Corner)
    // Completely non-wooden: matte iron spindles, brass finials, crisp white linens
    // =======================================================================
    // Located at X in [-2.4, -0.9], Z in [-2.2, -0.5]
    // 1. Metal base mattress support frame (Dark wrought-iron)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 0.44f, -1.35f), glm::vec3(1.40f, 0.12f, 1.70f), colIronMetal);

    // 2. Four Slender Cylindrical Iron Corner Posts with Polished Brass Ball Finials
    const glm::vec3 postPositions[4] = {
        glm::vec3(-2.30f, 0.95f, -2.15f), // Rear-left
        glm::vec3(-1.00f, 0.95f, -2.15f), // Rear-right
        glm::vec3(-2.30f, 0.72f, -0.55f), // Front-left
        glm::vec3(-1.00f, 0.72f, -0.55f)  // Front-right
    };
    const float postHeights[4] = { 1.35f, 1.35f, 0.85f, 0.85f };

    for (int p = 0; p < 4; ++p)
    {
        drawCylinderPart(shader, unitCylinder, rootMat,
                         postPositions[p], glm::vec3(0.06f, postHeights[p], 0.06f), colIronMetal);
        // Brass ball finials atop each post
        float ballY = postPositions[p].y + postHeights[p] * 0.5f + 0.04f;
        drawSpherePart(shader, unitSphere, rootMat,
                       glm::vec3(postPositions[p].x, ballY, postPositions[p].z),
                       glm::vec3(0.08f, 0.08f, 0.08f), colBrass);
    }

    // 3. Arched Iron Headboard Spindles (Rear wall)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 1.50f, -2.15f), glm::vec3(1.30f, 0.05f, 0.05f), colIronMetal);
    for (int s = 0; s < 5; ++s)
    {
        float sX = -2.10f + (float)s * 0.225f;
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(sX, 1.05f, -2.15f), glm::vec3(0.03f, 0.88f, 0.03f), colIronMetal);
    }

    // 4. Low Iron Footboard Spindles (Front edge)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 1.05f, -0.55f), glm::vec3(1.30f, 0.05f, 0.05f), colIronMetal);
    for (int s = 0; s < 5; ++s)
    {
        float sX = -2.10f + (float)s * 0.225f;
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(sX, 0.75f, -0.55f), glm::vec3(0.03f, 0.55f, 0.03f), colIronMetal);
    }

    // 5. Crisp Clean White Mattress
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 0.62f, -1.35f), glm::vec3(1.28f, 0.26f, 1.55f), colMattress);

    // 6. Luxury Deep Crimson Quilt / Duvet (Covering lower section of the bed)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 0.72f, -1.15f), glm::vec3(1.30f, 0.16f, 1.15f), colDuvet);

    // 7. Folded Cream Linen Bed Runner
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.65f, 0.81f, -0.80f), glm::vec3(1.32f, 0.03f, 0.35f), colRunner);

    // 8. Two Plush White Pillows
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.95f, 0.80f, -1.90f), glm::vec3(0.50f, 0.14f, 0.38f), colPillow);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.35f, 0.80f, -1.90f), glm::vec3(0.50f, 0.14f, 0.38f), colPillow);

    // Beside Nightstand (Painted cream with brass knob)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.62f, 0.58f, -2.05f), glm::vec3(0.42f, 0.48f, 0.42f), colPaintedCream);
    drawSpherePart(shader, unitSphere, rootMat,
                   glm::vec3(-0.62f, 0.58f, -1.82f), glm::vec3(0.04f, 0.04f, 0.04f), colBrass);
    // Nightstand candle
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(-0.62f, 0.88f, -2.05f), glm::vec3(0.05f, 0.12f, 0.05f), glm::vec3(0.95f, 0.94f, 0.90f));
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.62f, 0.98f, -2.05f), glm::vec3(0.03f, 0.04f, 0.03f),
                 colLanternGlow, 0.0f, 0.0f, 0.0f, true, colLanternGlow * 1.5f);

    // =======================================================================
    // B. OPEN HEARTH ZONE (Big chair in front of fireplace has been REMOVED!)
    // Replaced by a warm, fireside woven wool runner allowing complete free access & clear view
    // =======================================================================
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.05f, 0.365f, -0.60f), glm::vec3(0.95f, 0.012f, 1.85f), colRugWine);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.05f, 0.366f, -0.60f), glm::vec3(1.02f, 0.010f, 1.95f), colRugBorder);

    // =======================================================================
    // C. PAINTED STONE & IRON DINING / STUDY TABLE & CHAIRS (Front-Left Area)
    // Non-wooden: polished stone/cream tabletop with wrought-iron legs and upholstered chairs
    // =======================================================================
    // Tabletop (1.45m x 0.95m, height 0.76m) - Clean polished cream/stone finish
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.35f, 0.76f, 0.70f), glm::vec3(1.45f, 0.08f, 0.95f), colPaintedCream);

    // 4 Wrought-Iron Table Legs
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.95f, 0.38f, 0.35f), glm::vec3(0.05f, 0.72f, 0.05f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-0.75f, 0.38f, 0.35f), glm::vec3(0.05f, 0.72f, 0.05f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.95f, 0.38f, 1.05f), glm::vec3(0.05f, 0.72f, 0.05f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-0.75f, 0.38f, 1.05f), glm::vec3(0.05f, 0.72f, 0.05f), colIronMetal);

    // Chair 1 (North side of table: Z = 0.15) - Iron frame with fabric cushion
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-1.35f, 0.44f, 0.15f), glm::vec3(0.42f, 0.06f, 0.40f), colChairCushion);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-1.35f, 0.76f, -0.03f), glm::vec3(0.40f, 0.60f, 0.04f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.52f, 0.22f, 0.15f), glm::vec3(0.04f, 0.42f, 0.04f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.18f, 0.22f, 0.15f), glm::vec3(0.04f, 0.42f, 0.04f), colIronMetal);

    // Chair 2 (South side of table: Z = 1.25) - Iron frame with fabric cushion
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-1.35f, 0.44f, 1.25f), glm::vec3(0.42f, 0.06f, 0.40f), colChairCushion);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(-1.35f, 0.76f, 1.43f), glm::vec3(0.40f, 0.60f, 0.04f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.52f, 0.22f, 1.25f), glm::vec3(0.04f, 0.42f, 0.04f), colIronMetal);
    drawCylinderPart(shader, unitCylinder, rootMat, glm::vec3(-1.18f, 0.22f, 1.25f), glm::vec3(0.04f, 0.42f, 0.04f), colIronMetal);

    // Tabletop Ornaments:
    // Open Journal
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.45f, 0.81f, 0.65f), glm::vec3(0.36f, 0.02f, 0.26f), glm::vec3(0.95f, 0.92f, 0.85f));
    // Ceramic Coffee Cup
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(-1.10f, 0.82f, 0.85f), glm::vec3(0.09f, 0.12f, 0.09f), glm::vec3(0.92f, 0.90f, 0.88f));
    // Brass Tabletop Oil Lantern
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(-1.05f, 0.80f, 0.50f), glm::vec3(0.12f, 0.08f, 0.12f), colBrass);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-1.05f, 0.94f, 0.50f), glm::vec3(0.10f, 0.16f, 0.10f),
                 colLanternGlow, 0.0f, 0.0f, 0.0f, true, colLanternGlow * 1.6f);

    // =======================================================================
    // D. WROUGHT-IRON & IVORY WALL BOOKSHELF (Back wall: X = 0.15, Z = -2.30)
    // Non-wooden: black iron frame with painted ivory shelves and colorful tomes
    // =======================================================================
    // Outer iron frame
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.15f, 1.35f, -2.30f), glm::vec3(1.45f, 1.95f, 0.35f), colIronMetal);
    // Inset backing
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.15f, 1.35f, -2.25f), glm::vec3(1.30f, 1.80f, 0.28f), colPaintedCream * 0.80f);

    // 3 Painted ivory horizontal shelves
    drawCubePart(shader, unitCube, rootMat, glm::vec3(0.15f, 0.90f, -2.25f), glm::vec3(1.30f, 0.05f, 0.28f), colPaintedCream);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(0.15f, 1.40f, -2.25f), glm::vec3(1.30f, 0.05f, 0.28f), colPaintedCream);
    drawCubePart(shader, unitCube, rootMat, glm::vec3(0.15f, 1.90f, -2.25f), glm::vec3(1.30f, 0.05f, 0.28f), colPaintedCream);

    // Colorful book rows on shelves:
    // Shelf 1 (Bottom): Emerald green tomes
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.00f, 0.65f, -2.25f), glm::vec3(0.85f, 0.38f, 0.22f), glm::vec3(0.18f, 0.42f, 0.26f));
    // Shelf 2 (Middle): Crimson red & gold leather tomes
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.25f, 1.15f, -2.25f), glm::vec3(0.95f, 0.36f, 0.22f), glm::vec3(0.68f, 0.18f, 0.16f));
    // Shelf 3 (Top): Royal navy tomes + rolled parchment scrolls
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.10f, 1.65f, -2.25f), glm::vec3(0.70f, 0.34f, 0.22f), glm::vec3(0.16f, 0.28f, 0.58f));
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.60f, 1.55f, -2.25f), glm::vec3(0.10f, 0.28f, 0.10f), glm::vec3(0.92f, 0.88f, 0.78f), 0.0f, 0.0f, 85.0f);

    // =======================================================================
    // E. CENTER ROOM WOVEN WOOL TAPESTRY RUG
    // =======================================================================
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.15f, 0.375f, 0.10f), glm::vec3(2.20f, 0.015f, 1.70f), colRugWine);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(-0.15f, 0.376f, 0.10f), glm::vec3(2.32f, 0.012f, 1.82f), colRugBorder);

    // =======================================================================
    // F. CEILING RAFTERS & WROUGHT-IRON CHANDELIER LANTERN
    // =======================================================================
    const float beamZ[3] = { -1.50f, 0.0f, 1.50f };
    for (int i = 0; i < 3; ++i)
    {
        drawCubePart(shader, unitCube, rootMat,
                     glm::vec3(0.0f, 3.05f, beamZ[i]), glm::vec3(5.20f, 0.18f, 0.20f), colIronMetal);
    }

    // Wrought Iron Chain & Chandelier
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 2.75f, 0.0f), glm::vec3(0.03f, 0.50f, 0.03f), colIronMetal);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 2.45f, 0.0f), glm::vec3(0.24f, 0.06f, 0.24f), colIronMetal);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 2.30f, 0.0f), glm::vec3(0.18f, 0.24f, 0.18f),
                 colLanternGlow, 0.0f, 0.0f, 0.0f, true, colLanternGlow * 1.8f);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 2.16f, 0.0f), glm::vec3(0.24f, 0.04f, 0.24f), colIronMetal);
}

// ---------------------------------------------------------------------------
// Backward-Compatible Overloads
// ---------------------------------------------------------------------------
void Cabin::draw(const Shader& shader,
                 const Cube& unitCube,
                 const TriangularPrism& unitPrism,
                 const Cylinder& unitCylinder) const
{
    static const Sphere defaultSphere(1.0f, 16, 12);
    static const Pyramid defaultPyramid;
    draw(shader, unitCube, unitPrism, unitCylinder, defaultSphere, defaultPyramid);
}

void Cabin::draw(const Shader& shader,
                 const Cube& unitCube,
                 const TriangularPrism& unitPrism) const
{
    static const Cylinder defaultCylinder;
    static const Sphere defaultSphere(1.0f, 16, 12);
    static const Pyramid defaultPyramid;
    draw(shader, unitCube, unitPrism, defaultCylinder, defaultSphere, defaultPyramid);
}

void Cabin::draw(const Shader& shader,
                 const Cube& unitCube) const
{
    static const TriangularPrism defaultPrism;
    static const Cylinder defaultCylinder;
    static const Sphere defaultSphere(1.0f, 16, 12);
    static const Pyramid defaultPyramid;
    draw(shader, unitCube, defaultPrism, defaultCylinder, defaultSphere, defaultPyramid);
}

// ---------------------------------------------------------------------------
// 6. Architectural Semi-Transparent Glass Windows (Front, Lake-side, and Attic)
// ---------------------------------------------------------------------------
void Cabin::drawWindowsGlass(const Shader& shader,
                             const Cube& unitCube,
                             const glm::mat4& rootMat) const
{
    const glm::vec3 colGlass(0.72f, 0.88f, 0.96f); // Clean, crystal clear glass with faint atmospheric tint

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.setBool("isGlass", true);
    shader.setFloat("materialAlpha", 0.35f);
    shader.setFloat("shininess", 120.0f);
    shader.setBool("isEmissive", false);

    // Front Window Glass Pane (facing campsite & porch)
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(1.30f, 1.65f, 2.41f), glm::vec3(1.02f, 1.10f, 0.02f), colGlass);

    // Lake-Facing Side Window Glass Pane
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(2.61f, 1.65f, 0.60f), glm::vec3(0.02f, 1.10f, 1.02f), colGlass);

    // Attic Gable Window Glass Pane
    shader.setFloat("materialAlpha", 0.40f);
    drawCubePart(shader, unitCube, rootMat,
                 glm::vec3(0.0f, 3.85f, 2.84f), glm::vec3(0.60f, 0.54f, 0.02f), colGlass);

    shader.setBool("isGlass", false);
    shader.setFloat("materialAlpha", 1.0f);
    glDisable(GL_BLEND);
}

