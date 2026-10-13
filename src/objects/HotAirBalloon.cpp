#include "objects/HotAirBalloon.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

HotAirBalloon::HotAirBalloon(const glm::vec3& startPos, float startRotY, float scale)
    : m_position(startPos),
      m_rotationY(startRotY),
      m_scaleFactor(scale),
      m_forwardSpeed(0.0f),
      m_verticalSpeed(0.0f),
      m_maxForwardSpeed(6.0f),
      m_maxVerticalSpeed(4.5f),
      m_turnSpeed(35.0f),
      m_isBurning(false),
      m_flameFlicker(1.0f),
      m_currentBobY(0.0f),
      m_swayPitch(0.0f),
      m_swayRoll(0.0f),
      m_totalTime(0.0f)
{
}

void HotAirBalloon::setPosition(const glm::vec3& pos) { m_position = pos; }
void HotAirBalloon::setRotation(float rotY)           { m_rotationY = rotY; }
void HotAirBalloon::setScale(float scale)             { m_scaleFactor = scale; }

const glm::vec3& HotAirBalloon::getPosition() const   { return m_position; }
float HotAirBalloon::getRotation() const              { return m_rotationY; }
float HotAirBalloon::getScale() const                 { return m_scaleFactor; }
float HotAirBalloon::getForwardSpeed() const          { return m_forwardSpeed; }
float HotAirBalloon::getVerticalSpeed() const         { return m_verticalSpeed; }
bool HotAirBalloon::isBurnerActive() const            { return m_isBurning; }

// ---------------------------------------------------------------------------
// Kinematic Flight & Input Integration
// ---------------------------------------------------------------------------
void HotAirBalloon::processInput(bool forward, bool backward,
                                 bool turnLeft, bool turnRight,
                                 bool ascend, bool descend,
                                 bool boost, float dt)
{
    // 1. Vertical Altitude / Burner Thrust Control
    float targetVert = 0.0f;
    if (ascend)
    {
        targetVert += (boost ? 7.2f : m_maxVerticalSpeed);
        m_isBurning = true;
    }
    else if (descend)
    {
        targetVert -= (boost ? 5.8f : 3.6f);
        m_isBurning = false;
    }
    else
    {
        targetVert = 0.0f;
        m_isBurning = false;
    }

    float vertAccel = 2.6f;
    if (m_verticalSpeed < targetVert)
    {
        m_verticalSpeed = std::min(m_verticalSpeed + vertAccel * dt, targetVert);
    }
    else if (m_verticalSpeed > targetVert)
    {
        m_verticalSpeed = std::max(m_verticalSpeed - vertAccel * dt, targetVert);
    }

    // 2. Horizontal Forward / Backward Cruising Control
    float targetHoriz = 0.0f;
    if (forward)  targetHoriz += (boost ? 8.8f : m_maxForwardSpeed);
    if (backward) targetHoriz -= 3.2f;

    float horizAccel = 2.2f;
    if (m_forwardSpeed < targetHoriz)
    {
        m_forwardSpeed = std::min(m_forwardSpeed + horizAccel * dt, targetHoriz);
    }
    else if (m_forwardSpeed > targetHoriz)
    {
        m_forwardSpeed = std::max(m_forwardSpeed - horizAccel * dt, targetHoriz);
    }

    // 3. Yaw Steering Control
    float effectiveTurnSpeed = boost ? 48.0f : m_turnSpeed;
    if (turnLeft)
    {
        m_rotationY += effectiveTurnSpeed * dt;
    }
    if (turnRight)
    {
        m_rotationY -= effectiveTurnSpeed * dt;
    }

    // 4. World Displacement (Heading 0 along +Z)
    float rad = glm::radians(m_rotationY);
    m_position.x += m_forwardSpeed * std::sin(rad) * dt;
    m_position.z += m_forwardSpeed * std::cos(rad) * dt;
    m_position.y += m_verticalSpeed * dt;

    // Apply boundary limits
    constrainToBounds();
}

// ---------------------------------------------------------------------------
// Atmospheric Harmonics & Aerodynamic Swaying
// ---------------------------------------------------------------------------
void HotAirBalloon::update(float dt, float totalTime)
{
    m_totalTime = totalTime;

    // 1. Buoyant atmospheric vertical bobbing: y(t) = A * sin(omega * t)
    m_currentBobY = 0.08f * std::sin(1.2f * totalTime);

    // 2. Dynamic flight pitch (momentum forward tilt + wind turbulence)
    float motionPitch = (m_forwardSpeed / m_maxForwardSpeed) * -3.5f;
    float windPitch = 1.2f * std::sin(1.5f * totalTime + 0.3f);
    m_swayPitch = motionPitch + windPitch;

    // 3. Roll sway (transverse rocking in mountain currents)
    m_swayRoll = 1.4f * std::cos(0.9f * totalTime);

    // 4. Turbulent burner flame flicker
    m_flameFlicker = 1.0f + 0.18f * std::sin(18.0f * totalTime) + 0.12f * std::cos(29.0f * totalTime);
}

// ---------------------------------------------------------------------------
// World Boundary Constraint
// ---------------------------------------------------------------------------
void HotAirBalloon::constrainToBounds()
{
    // Vertical limits: basket base stays safely above water/terrain (min 1.5m), up to cloud ceiling (52m)
    m_position.y = glm::clamp(m_position.y, 1.5f, 52.0f);

    // Horizontal bounds: keeps craft within the panoramic MoonLake basin
    m_position.x = glm::clamp(m_position.x, -45.0f, 45.0f);
    m_position.z = glm::clamp(m_position.z, -50.0f, 45.0f);
}

// ---------------------------------------------------------------------------
// Compound Hierarchical Root Transformation Matrix
// ---------------------------------------------------------------------------
glm::mat4 HotAirBalloon::getRootMatrix() const
{
    glm::mat4 root(1.0f);
    glm::vec3 animatedPos = m_position + glm::vec3(0.0f, m_currentBobY, 0.0f);
    root = glm::translate(root, animatedPos);
    root = glm::rotate(root, glm::radians(m_rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::rotate(root, glm::radians(m_swayPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    root = glm::rotate(root, glm::radians(m_swayRoll), glm::vec3(0.0f, 0.0f, 1.0f));
    root = glm::scale(root, glm::vec3(m_scaleFactor));
    return root;
}

// ---------------------------------------------------------------------------
// Third-Person Follow Camera Target & Position
// ---------------------------------------------------------------------------
glm::vec3 HotAirBalloon::getCameraFollowPosition() const
{
    // Elevated behind the balloon along its heading, looking over the basket and envelope
    float rad = glm::radians(m_rotationY);
    glm::vec3 localCamOffset(0.0f, 5.2f, -14.5f);

    glm::vec3 worldOffset(
        localCamOffset.x * std::cos(rad) + localCamOffset.z * std::sin(rad),
        localCamOffset.y,
        -localCamOffset.x * std::sin(rad) + localCamOffset.z * std::cos(rad)
    );

    return m_position + glm::vec3(0.0f, m_currentBobY, 0.0f) + worldOffset;
}

glm::vec3 HotAirBalloon::getCameraLookTarget() const
{
    // Look slightly ahead through the balloon toward the panoramic horizon
    float rad = glm::radians(m_rotationY);
    glm::vec3 localLookOffset(0.0f, 4.0f, 8.0f);

    glm::vec3 worldOffset(
        localLookOffset.x * std::cos(rad) + localLookOffset.z * std::sin(rad),
        localLookOffset.y,
        -localLookOffset.x * std::sin(rad) + localLookOffset.z * std::cos(rad)
    );

    return m_position + glm::vec3(0.0f, m_currentBobY, 0.0f) + worldOffset;
}

// ---------------------------------------------------------------------------
// First-Person Passenger Gondola View
// ---------------------------------------------------------------------------
glm::vec3 HotAirBalloon::getBasketCameraPosition() const
{
    float rad = glm::radians(m_rotationY);
    glm::vec3 localEye(0.0f, 1.25f, 0.2f); // Eye height standing inside the basket

    glm::vec3 worldOffset(
        localEye.x * std::cos(rad) + localEye.z * std::sin(rad),
        localEye.y,
        -localEye.x * std::sin(rad) + localEye.z * std::cos(rad)
    );

    return m_position + glm::vec3(0.0f, m_currentBobY, 0.0f) + worldOffset;
}

glm::vec3 HotAirBalloon::getBasketCameraLookTarget() const
{
    float rad = glm::radians(m_rotationY);
    glm::vec3 localLook(0.0f, 1.15f, 12.0f); // Looking out forward over the wicker railing

    glm::vec3 worldOffset(
        localLook.x * std::cos(rad) + localLook.z * std::sin(rad),
        localLook.y,
        -localLook.x * std::sin(rad) + localLook.z * std::cos(rad)
    );

    return m_position + glm::vec3(0.0f, m_currentBobY, 0.0f) + worldOffset;
}

// ---------------------------------------------------------------------------
// Unit Primitive Draw Helpers
// ---------------------------------------------------------------------------
void HotAirBalloon::drawPart(const Shader& shader,
                             const Cube& unitCube,
                             const glm::mat4& parentMat,
                             const glm::vec3& localOffset,
                             const glm::vec3& localScale,
                             const glm::vec3& color,
                             float rotY, float rotX, float rotZ,
                             bool emissive,
                             const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitCube.draw();
}

void HotAirBalloon::drawCylinderPart(const Shader& shader,
                                     const Cylinder& unitCylinder,
                                     const glm::mat4& parentMat,
                                     const glm::vec3& localOffset,
                                     const glm::vec3& localScale,
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

void HotAirBalloon::drawSpherePart(const Shader& shader,
                                   const Sphere& unitSphere,
                                   const glm::mat4& parentMat,
                                   const glm::vec3& localOffset,
                                   const glm::vec3& localScale,
                                   const glm::vec3& color,
                                   bool emissive,
                                   const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitSphere.draw();
}

void HotAirBalloon::drawPyramidPart(const Shader& shader,
                                    const Pyramid& unitPyramid,
                                    const glm::mat4& parentMat,
                                    const glm::vec3& localOffset,
                                    const glm::vec3& localScale,
                                    const glm::vec3& color,
                                    float rotY, float rotX, float rotZ,
                                    bool emissive,
                                    const glm::vec3& emitColor) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, localOffset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, localScale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", emissive);
    if (emissive)
    {
        shader.setVec3("emissionColor", emitColor);
    }

    unitPyramid.draw();
}

// ---------------------------------------------------------------------------
// Main Draw Routine
// ---------------------------------------------------------------------------
void HotAirBalloon::draw(const Shader& shader,
                         const Cube& unitCube,
                         const Cylinder& unitCylinder,
                         const Sphere& unitSphere,
                         const Pyramid& unitPyramid) const
{
    glm::mat4 rootMat = getRootMatrix();

    // 1. Passenger Gondola Basket (Wicker, leather rails, sandbags, lantern)
    drawBasket(shader, unitCube, unitCylinder, unitSphere, rootMat);

    // 2. Burner Assembly & Dynamic Emissive Flame
    drawBurner(shader, unitCube, unitCylinder, unitPyramid, unitSphere, rootMat);

    // 3. Steel / Nylon Suspension Rigging Cables
    drawRigging(shader, unitCylinder, rootMat);

    // 4. Multi-Segment Striped Balloon Canopy (Crimson, Royal Blue, Gold, Ivory)
    drawEnvelope(shader, unitCylinder, unitSphere, rootMat);
}

// ---------------------------------------------------------------------------
// 1. Wicker Passenger Basket (Gondola)
// ---------------------------------------------------------------------------
void HotAirBalloon::drawBasket(const Shader& shader,
                               const Cube& unitCube,
                               const Cylinder& unitCylinder,
                               const Sphere& unitSphere,
                               const glm::mat4& rootMat) const
{
    // Color Palette
    const glm::vec3 wickerFloorColor(0.42f, 0.28f, 0.16f);
    const glm::vec3 wickerBodyColor(0.60f, 0.42f, 0.24f);
    const glm::vec3 wickerWeaveAccent(0.50f, 0.34f, 0.19f);
    const glm::vec3 leatherRimColor(0.26f, 0.14f, 0.08f);
    const glm::vec3 supportPoleColor(0.22f, 0.22f, 0.24f);
    const glm::vec3 sandbagColor(0.76f, 0.68f, 0.48f);
    const glm::vec3 bronzeMetalColor(0.48f, 0.38f, 0.20f);
    const glm::vec3 lanternGlowColor(1.0f, 0.88f, 0.42f);

    // A. Floor Base Plank
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.06f, 0.0f),
             glm::vec3(1.85f, 0.12f, 1.85f),
             wickerFloorColor);

    // B. Four Wicker Perimeter Walls
    // Front Wall (+Z)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.55f, 0.88f),
             glm::vec3(1.85f, 0.88f, 0.10f),
             wickerBodyColor);
    // Back Wall (-Z)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.55f, -0.88f),
             glm::vec3(1.85f, 0.88f, 0.10f),
             wickerBodyColor);
    // Left Wall (-X)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.88f, 0.55f, 0.0f),
             glm::vec3(0.10f, 0.88f, 1.66f),
             wickerBodyColor);
    // Right Wall (+X)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.88f, 0.55f, 0.0f),
             glm::vec3(0.10f, 0.88f, 1.66f),
             wickerBodyColor);

    // Horizontal Wicker Weave Accent Belts (Midway up the basket)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.52f, 0.90f),
             glm::vec3(1.90f, 0.14f, 0.08f),
             wickerWeaveAccent);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.52f, -0.90f),
             glm::vec3(1.90f, 0.14f, 0.08f),
             wickerWeaveAccent);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.90f, 0.52f, 0.0f),
             glm::vec3(0.08f, 0.14f, 1.72f),
             wickerWeaveAccent);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.90f, 0.52f, 0.0f),
             glm::vec3(0.08f, 0.14f, 1.72f),
             wickerWeaveAccent);

    // C. Padded Leather Protective Handrails Along Top Rim
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 1.02f, 0.88f),
             glm::vec3(1.95f, 0.08f, 0.15f),
             leatherRimColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 1.02f, -0.88f),
             glm::vec3(1.95f, 0.08f, 0.15f),
             leatherRimColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.88f, 1.02f, 0.0f),
             glm::vec3(0.15f, 0.08f, 1.75f),
             leatherRimColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.88f, 1.02f, 0.0f),
             glm::vec3(0.15f, 0.08f, 1.75f),
             leatherRimColor);

    // D. Four Heavy Upright Burner Support Corner Posts
    const float postX[4] = { -0.78f,  0.78f, -0.78f,  0.78f };
    const float postZ[4] = { -0.78f, -0.78f,  0.78f,  0.78f };
    for (int i = 0; i < 4; ++i)
    {
        // Cylinder height = 1.0 from base, scale height by 1.25
        drawCylinderPart(shader, unitCylinder, rootMat,
                         glm::vec3(postX[i], 0.98f, postZ[i]),
                         glm::vec3(0.06f, 1.25f, 0.06f),
                         supportPoleColor);
    }

    // E. Four Hanging Sandbags (Ballast weights)
    const glm::vec3 sandbagPositions[4] = {
        glm::vec3(-0.55f, 0.44f,  0.98f),
        glm::vec3( 0.55f, 0.44f,  0.98f),
        glm::vec3(-0.55f, 0.44f, -0.98f),
        glm::vec3( 0.55f, 0.44f, -0.98f)
    };
    for (int i = 0; i < 4; ++i)
    {
        drawSpherePart(shader, unitSphere, rootMat,
                       sandbagPositions[i],
                       glm::vec3(0.11f, 0.16f, 0.10f),
                       sandbagColor);
    }

    // F. Navigational Lantern (Hanging on front-right corner post)
    // Bronze support bracket
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.84f, 1.28f, 0.78f),
             glm::vec3(0.12f, 0.03f, 0.03f),
             bronzeMetalColor);
    // Lantern frame cap & bottom
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.93f, 1.34f, 0.78f),
             glm::vec3(0.14f, 0.04f, 0.14f),
             bronzeMetalColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.93f, 1.16f, 0.78f),
             glm::vec3(0.14f, 0.04f, 0.14f),
             bronzeMetalColor);
    // Glowing lantern glass core
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.93f, 1.25f, 0.78f),
             glm::vec3(0.10f, 0.14f, 0.10f),
             lanternGlowColor,
             0.0f, 0.0f, 0.0f,
             true, lanternGlowColor * 1.4f);
}

// ---------------------------------------------------------------------------
// 2. Burner Assembly & Dynamic Emissive Flame
// ---------------------------------------------------------------------------
void HotAirBalloon::drawBurner(const Shader& shader,
                               const Cube& unitCube,
                               const Cylinder& unitCylinder,
                               const Pyramid& unitPyramid,
                               const Sphere& unitSphere,
                               const glm::mat4& rootMat) const
{
    const glm::vec3 darkMetalColor(0.24f, 0.24f, 0.26f);
    const glm::vec3 brassCoilColor(0.88f, 0.74f, 0.26f);

    // A. Cross Support Girders & Mount Plate
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 2.22f, 0.0f),
             glm::vec3(1.60f, 0.05f, 0.05f),
             darkMetalColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 2.22f, 0.0f),
             glm::vec3(0.05f, 0.05f, 1.60f),
             darkMetalColor);
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 2.24f, 0.0f),
             glm::vec3(0.55f, 0.06f, 0.55f),
             darkMetalColor);

    // B. Dual High-Pressure Brass Vaporization Coils
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(-0.14f, 2.26f, 0.0f),
                     glm::vec3(0.20f, 0.14f, 0.20f),
                     brassCoilColor);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3( 0.14f, 2.26f, 0.0f),
                     glm::vec3(0.20f, 0.14f, 0.20f),
                     brassCoilColor);

    // C. Dynamic Emissive Flame Jets
    if (m_isBurning)
    {
        // Roaring high-thrust burner flame shooting up into the throat
        float flameHeight = 1.95f * m_flameFlicker;
        float flameRadius = 0.55f * m_flameFlicker;

        // Outer Incandescent Fiery Orange Mantle
        const glm::vec3 outerFlameColor(1.0f, 0.42f, 0.05f);
        const glm::vec3 outerEmitColor(1.0f, 0.50f, 0.10f);
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(0.0f, 2.40f, 0.0f),
                        glm::vec3(flameRadius, flameHeight, flameRadius),
                        outerFlameColor,
                        0.0f, 0.0f, 0.0f,
                        true, outerEmitColor * 2.2f);

        // Core White-Hot Blue-Gold Heart
        const glm::vec3 coreFlameColor(1.0f, 0.95f, 0.70f);
        const glm::vec3 coreEmitColor(1.0f, 1.0f, 0.90f);
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(0.0f, 2.40f, 0.0f),
                        glm::vec3(flameRadius * 0.45f, flameHeight * 0.65f, flameRadius * 0.45f),
                        coreFlameColor,
                        0.0f, 0.0f, 0.0f,
                        true, coreEmitColor * 2.8f);

        // Emissive light sphere centered at the burner nozzle
        drawSpherePart(shader, unitSphere, rootMat,
                       glm::vec3(0.0f, 2.50f, 0.0f),
                       glm::vec3(0.22f),
                       glm::vec3(1.0f, 0.9f, 0.5f),
                       true, glm::vec3(1.0f, 0.8f, 0.3f) * 2.0f);
    }
    else
    {
        // Gentle resting pilot flame
        float pilotHeight = 0.45f * m_flameFlicker;
        const glm::vec3 pilotColor(1.0f, 0.60f, 0.15f);
        drawPyramidPart(shader, unitPyramid, rootMat,
                        glm::vec3(0.0f, 2.40f, 0.0f),
                        glm::vec3(0.18f, pilotHeight, 0.18f),
                        pilotColor,
                        0.0f, 0.0f, 0.0f,
                        true, pilotColor * 1.5f);
    }
}

// ---------------------------------------------------------------------------
// 3. Steel / Nylon Suspension Rigging Lines
// ---------------------------------------------------------------------------
void HotAirBalloon::drawRigging(const Shader& shader,
                                const Cylinder& unitCylinder,
                                const glm::mat4& rootMat) const
{
    const glm::vec3 cableColor(0.25f, 0.25f, 0.28f);

    // 4 rigging cables connecting basket corners to envelope throat collar
    const glm::vec3 basketCorners[4] = {
        glm::vec3(-0.78f, 1.02f, -0.78f),
        glm::vec3( 0.78f, 1.02f, -0.78f),
        glm::vec3(-0.78f, 1.02f,  0.78f),
        glm::vec3( 0.78f, 1.02f,  0.78f)
    };

    const glm::vec3 throatCorners[4] = {
        glm::vec3(-0.62f, 2.75f, -0.62f),
        glm::vec3( 0.62f, 2.75f, -0.62f),
        glm::vec3(-0.62f, 2.75f,  0.62f),
        glm::vec3( 0.62f, 2.75f,  0.62f)
    };

    for (int i = 0; i < 4; ++i)
    {
        glm::vec3 start = basketCorners[i];
        glm::vec3 end = throatCorners[i];
        glm::vec3 delta = end - start;
        float length = glm::length(delta);
        if (length < 0.001f) continue;

        glm::vec3 dir = delta / length;

        // Rotation from default cylinder axis (0, 1, 0) to dir
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::mat4 cableMat = rootMat;
        cableMat = glm::translate(cableMat, start);

        float dot = glm::clamp(glm::dot(up, dir), -1.0f, 1.0f);
        if (dot < 0.9999f)
        {
            if (dot > -0.9999f)
            {
                glm::vec3 axis = glm::normalize(glm::cross(up, dir));
                float angle = std::acos(dot);
                cableMat = glm::rotate(cableMat, angle, axis);
            }
            else
            {
                cableMat = glm::rotate(cableMat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }

        cableMat = glm::scale(cableMat, glm::vec3(0.024f, length, 0.024f));

        shader.setMat4("model", cableMat);
        shader.setVec3("objectColor", cableColor);
        shader.setBool("isEmissive", false);

        unitCylinder.draw();
    }
}

// ---------------------------------------------------------------------------
// 4. Multi-Segment Striped Balloon Canopy (Crimson, Royal Blue, Gold, Ivory)
// ---------------------------------------------------------------------------
void HotAirBalloon::drawEnvelope(const Shader& shader,
                                 const Cylinder& unitCylinder,
                                 const Sphere& unitSphere,
                                 const glm::mat4& rootMat) const
{
    // Color Palette
    const glm::vec3 crimsonRed(0.86f, 0.16f, 0.20f);
    const glm::vec3 royalBlue(0.12f, 0.40f, 0.86f);
    const glm::vec3 warmGold(0.98f, 0.78f, 0.18f);
    const glm::vec3 ivoryWhite(0.95f, 0.94f, 0.90f);
    const glm::vec3 throatDark(0.20f, 0.20f, 0.22f);
    const glm::vec3 topVentColor(0.18f, 0.20f, 0.24f);

    // A. Heat-Resistant Throat Collar Ring (Bottom Aperture)
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 2.70f, 0.0f),
                     glm::vec3(1.35f, 0.28f, 1.35f),
                     throatDark);
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 2.95f, 0.0f),
                     glm::vec3(1.48f, 0.08f, 1.48f),
                     warmGold);

    // B. Tapered Lower Skirt (Step-tapered rings transitioning upward)
    // Lower Cone Section (Crimson Red)
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 3.03f, 0.0f),
                     glm::vec3(2.05f, 0.75f, 2.05f),
                     crimsonRed);
    // Lower Gold Trim Ring
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 3.75f, 0.0f),
                     glm::vec3(2.65f, 0.08f, 2.65f),
                     warmGold);
    // Mid Skirt Flare (Royal Blue)
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 3.82f, 0.0f),
                     glm::vec3(3.25f, 0.85f, 3.25f),
                     royalBlue);
    // Mid Gold Trim Ring
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 4.65f, 0.0f),
                     glm::vec3(4.20f, 0.08f, 4.20f),
                     warmGold);
    // Upper Skirt Flare (Ivory White)
    drawCylinderPart(shader, unitCylinder, rootMat,
                     glm::vec3(0.0f, 4.72f, 0.0f),
                     glm::vec3(4.55f, 0.80f, 4.55f),
                     ivoryWhite);

    // C. Main Bulbous Aerodynamic Canopy
    // Canopy center is elevated at Y = 6.65m
    const glm::vec3 canopyCenter(0.0f, 6.65f, 0.0f);

    // 1. Primary Majestic Crimson Red Gasbag Sphere (Radius 3.15m)
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter,
                   glm::vec3(3.15f, 3.35f, 3.15f),
                   crimsonRed);

    // 2. Wide Equatorial Royal Blue Decorative Belt
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter,
                   glm::vec3(3.20f, 1.15f, 3.20f),
                   royalBlue);

    // 3. Central Gilded Equatorial Accent Ring
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter,
                   glm::vec3(3.24f, 0.32f, 3.24f),
                   warmGold);

    // 4. Upper Golden Latitude Ring
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter + glm::vec3(0.0f, 1.45f, 0.0f),
                   glm::vec3(2.72f, 0.32f, 2.72f),
                   warmGold);

    // 5. Lower Golden Latitude Ring
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter - glm::vec3(0.0f, 1.35f, 0.0f),
                   glm::vec3(2.72f, 0.32f, 2.72f),
                   warmGold);

    // 6. Upper Ivory White Crown Band
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter + glm::vec3(0.0f, 2.25f, 0.0f),
                   glm::vec3(1.95f, 0.70f, 1.95f),
                   ivoryWhite);

    // D. Top Parachute Vent Valve & Decorative Finial
    // Top Vent Disc (Flat dark disc at the very peak of the envelope)
    drawCylinderPart(shader, unitCylinder, rootMat,
                     canopyCenter + glm::vec3(0.0f, 3.30f, 0.0f),
                     glm::vec3(1.10f, 0.15f, 1.10f),
                     topVentColor);
    // Golden Finial Cap
    drawSpherePart(shader, unitSphere, rootMat,
                   canopyCenter + glm::vec3(0.0f, 3.48f, 0.0f),
                   glm::vec3(0.28f, 0.28f, 0.28f),
                   warmGold);
}
