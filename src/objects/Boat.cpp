#include "objects/Boat.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

Boat::Boat(const glm::vec3& startPos, float startRotY, float scale)
    : position(startPos),
      rotationY(startRotY),
      scaleFactor(scale),
      speed(0.0f),
      maxSpeed(4.5f),
      turnSpeed(45.0f),
      currentBobY(0.0f),
      currentPitch(0.0f),
      currentRoll(0.0f),
      oarSwingAngle(0.0f),
      totalTimeElapsed(0.0f)
{
}

void Boat::setPosition(const glm::vec3& pos) { position = pos; }
void Boat::setRotation(float rotY)           { rotationY = rotY; }
void Boat::setScale(float scale)             { scaleFactor = scale; }

const glm::vec3& Boat::getPosition() const   { return position; }
float Boat::getRotation() const              { return rotationY; }
float Boat::getScale() const                 { return scaleFactor; }
float Boat::getSpeed() const                 { return speed; }

// ---------------------------------------------------------------------------
// Movement Physics & Control: Frame-rate independent kinematic integration
// ---------------------------------------------------------------------------
void Boat::processInput(bool forward, bool backward, bool left, bool right, float dt)
{
    float targetSpeed = 0.0f;
    if (forward) targetSpeed += maxSpeed;
    if (backward) targetSpeed -= maxSpeed * 0.55f; // Slower reverse

    // Smooth inertia acceleration & deceleration
    float accelRate = (std::abs(targetSpeed) > std::abs(speed)) ? 3.0f : 2.0f;
    if (speed < targetSpeed)
    {
        speed = std::min(speed + accelRate * dt, targetSpeed);
    }
    else if (speed > targetSpeed)
    {
        speed = std::max(speed - accelRate * dt, targetSpeed);
    }

    // Steering turn rate (Rudder response proportional to motion + small stationary pivoting)
    float effectiveTurnSpeed = turnSpeed * (0.35f + 0.65f * (std::abs(speed) / maxSpeed));
    if (left)
    {
        rotationY += effectiveTurnSpeed * dt;
    }
    if (right)
    {
        rotationY -= effectiveTurnSpeed * dt;
    }

    // Forward Kinematics displacement:
    // Heading theta is rotationY (degrees). Forward vector in world space:
    // With heading 0 aligned along +Z: x' = x + v * sin(theta) * dt, z' = z + v * cos(theta) * dt
    float rad = glm::radians(rotationY);
    position.x += speed * std::sin(rad) * dt;
    position.z += speed * std::cos(rad) * dt;

    // Constrain position within navigable lake boundaries
    constrainToLake();
}

// ---------------------------------------------------------------------------
// Harmonic Wave Bobbing & Rolling Animation
// ---------------------------------------------------------------------------
void Boat::update(float dt, float totalTime)
{
    totalTimeElapsed = totalTime;

    // 1. Water Bobbing: y' = y0 + A * sin(omega * t)
    // Primary vertical wave harmonic: A = 0.024m, omega = 2.2 rad/s
    currentBobY = 0.024f * std::sin(2.2f * totalTime);

    // 2. Wave Rocking:
    // Pitch (longitudinal): tilt induced by speed acceleration + water wave rocking
    float wavePitch = 1.2f * std::sin(1.8f * totalTime + 0.5f);
    float motionPitch = (speed / maxSpeed) * -2.4f; // Bow lifts slightly during forward thrust
    currentPitch = wavePitch + motionPitch;

    // Roll (transverse): gentle side-to-side roll
    currentRoll = 1.6f * std::cos(1.4f * totalTime);

    // 3. Periodic Oar Swing:
    // When moving, oars row back and forth: theta_oar = A_oar * sin(omega_row * t)
    if (std::abs(speed) > 0.1f)
    {
        oarSwingAngle = 26.0f * std::sin(4.5f * totalTime);
    }
    else
    {
        // Gentle resting idle drift
        oarSwingAngle = 4.0f * std::sin(1.0f * totalTime);
    }
}

// ---------------------------------------------------------------------------
// Lake Boundary Constraint (Keep boat within water basin)
// ---------------------------------------------------------------------------
void Boat::constrainToLake()
{
    // Water surface level
    position.y = -1.14f;

    // Navigable lake bounds:
    // Lake center: (12.0, -1.18, -4.0)
    // Dock is located at West side: X ~ 1.3 to 2.4, Z ~ 6.0 to 10.0
    // North shore ~ Z: -22.0
    // South beach ~ Z: +16.5
    // East cliffs ~ X: +22.5
    // West shore  ~ X: +3.2 (near dock), +2.4 (north/south)

    // North-South boundary
    if (position.z < -22.0f) { position.z = -22.0f; speed = 0.0f; }
    // Allowed south navigation extends to Z = 18.2m to comfortably reach the southern autumn dock
    if (position.z >  18.2f) { position.z =  18.2f; speed = 0.0f; }

    // East boundary (Cliffs)
    if (position.x >  22.5f) { position.x =  22.5f; speed = 0.0f; }

    // West boundary (Dock and shoreline)
    float minX = (position.z >= 5.5f && position.z <= 10.5f) ? 3.6f : 2.4f;
    if (position.x < minX)
    {
        position.x = minX;
        speed = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Compound Hierarchical Root Transformation Matrix
// M_boat = T(P0 + (0, currentBobY, 0)) * R_y(rotationY) * R_x(currentPitch) * R_z(currentRoll) * S(scale)
// ---------------------------------------------------------------------------
glm::mat4 Boat::getRootMatrix() const
{
    glm::mat4 root(1.0f);
    glm::vec3 animatedPos = position + glm::vec3(0.0f, currentBobY, 0.0f);
    root = glm::translate(root, animatedPos);
    root = glm::rotate(root, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::rotate(root, glm::radians(currentPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    root = glm::rotate(root, glm::radians(currentRoll), glm::vec3(0.0f, 0.0f, 1.0f));
    root = glm::scale(root, glm::vec3(scaleFactor));
    return root;
}

// ---------------------------------------------------------------------------
// Third-Person Follow Camera Target & Position
// ---------------------------------------------------------------------------
glm::vec3 Boat::getCameraFollowPosition() const
{
    // Camera is elevated and placed behind the boat along its heading
    // Local camera offset: (0.0, 2.2, -5.2) -> behind stern, above water
    float rad = glm::radians(rotationY);
    glm::vec3 localCamOffset(0.0f, 2.2f, -5.2f);

    glm::vec3 worldOffset(
        localCamOffset.x * std::cos(rad) + localCamOffset.z * std::sin(rad),
        localCamOffset.y,
        -localCamOffset.x * std::sin(rad) + localCamOffset.z * std::cos(rad)
    );

    return position + glm::vec3(0.0f, currentBobY, 0.0f) + worldOffset;
}

glm::vec3 Boat::getCameraLookTarget() const
{
    // Look ahead of the boat toward the bow
    float rad = glm::radians(rotationY);
    glm::vec3 localLookOffset(0.0f, 0.6f, 3.5f);

    glm::vec3 worldOffset(
        localLookOffset.x * std::cos(rad) + localLookOffset.z * std::sin(rad),
        localLookOffset.y,
        -localLookOffset.x * std::sin(rad) + localLookOffset.z * std::cos(rad)
    );

    return position + glm::vec3(0.0f, currentBobY, 0.0f) + worldOffset;
}

// ---------------------------------------------------------------------------
// Transformation Helpers for Component Pieces
// ---------------------------------------------------------------------------
void Boat::drawPart(const Shader& shader,
                    const Cube& unitCube,
                    const glm::mat4& parentMat,
                    const glm::vec3& localOffset,
                    const glm::vec3& localScale,
                    const glm::vec3& color,
                    float rotY, float rotX, float rotZ,
                    bool emissive, const glm::vec3& emitColor) const
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

void Boat::drawTriangularPart(const Shader& shader,
                              const TriangularPrism& unitPrism,
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

    unitPrism.draw();
}

void Boat::drawCylinderPart(const Shader& shader,
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

// ---------------------------------------------------------------------------
// Hierarchical Component: Hull
// (Bottom planks, keel, flared port/starboard gunwales, bow stem, stern transom)
// ---------------------------------------------------------------------------
void Boat::drawHull(const Shader& shader, const Cube& unitCube, const TriangularPrism& unitPrism, const glm::mat4& rootMat) const
{
    // Wood palette matching rustic cabin & dock styling
    const glm::vec3 hullWoodDark(0.32f, 0.22f, 0.14f);   // Weathered outer hull
    const glm::vec3 hullWoodMid(0.40f, 0.28f, 0.18f);    // Hull side planks
    const glm::vec3 gunwaleWood(0.24f, 0.16f, 0.10f);    // Top rim / rub rail
    const glm::vec3 keelWood(0.18f, 0.12f, 0.08f);       // Keel

    // 1. Keel beam (Central longitudinal backbone running along bottom)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, -0.05f, 0.0f),
             glm::vec3(0.12f, 0.10f, 2.70f),
             keelWood);

    // 2. Bottom Hull Planking (Floor of the boat)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(1.00f, 0.06f, 2.45f),
             hullWoodDark);

    // 3. Port (Left) Hull Side - Flared outward by ~10 degrees for authentic boat hull lines
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.54f, 0.18f, 0.0f),
             glm::vec3(0.07f, 0.38f, 2.45f),
             hullWoodMid,
             0.0f, 0.0f, 11.0f);

    // Port Gunwale / Top rail
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.58f, 0.38f, 0.0f),
             glm::vec3(0.10f, 0.06f, 2.50f),
             gunwaleWood);

    // 4. Starboard (Right) Hull Side - Flared outward symmetrically
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.54f, 0.18f, 0.0f),
             glm::vec3(0.07f, 0.38f, 2.45f),
             hullWoodMid,
             0.0f, 0.0f, -11.0f);

    // Starboard Gunwale / Top rail
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.58f, 0.38f, 0.0f),
             glm::vec3(0.10f, 0.06f, 2.50f),
             gunwaleWood);

    // 5. Stern Transom (Flat back wall at -Z)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.18f, -1.22f),
             glm::vec3(1.08f, 0.38f, 0.08f),
             hullWoodDark,
             0.0f, -12.0f, 0.0f); // Tilted slightly outward

    // Stern top cap
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.38f, -1.24f),
             glm::vec3(1.12f, 0.06f, 0.10f),
             gunwaleWood);

    // 6. Bow Taper & Stem Post (Sharp front at +Z)
    // Left tapered bow plank angled inwards toward the bow tip
    drawPart(shader, unitCube, rootMat,
             glm::vec3(-0.29f, 0.18f, 1.42f),
             glm::vec3(0.07f, 0.38f, 0.58f),
             hullWoodMid,
             -24.0f, 0.0f, 11.0f);

    // Right tapered bow plank angled inwards toward the bow tip
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.29f, 0.18f, 1.42f),
             glm::vec3(0.07f, 0.38f, 0.58f),
             hullWoodMid,
             24.0f, 0.0f, -11.0f);

    // Bow Stem Post (Vertical leading edge)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.22f, 1.68f),
             glm::vec3(0.10f, 0.44f, 0.12f),
             keelWood,
             0.0f, 14.0f, 0.0f);

    // Triangular bow breast hook / deck plate capping the front tip
    drawTriangularPart(shader, unitPrism, rootMat,
                       glm::vec3(0.0f, 0.32f, 1.35f),
                       glm::vec3(0.70f, 0.08f, 0.65f),
                       hullWoodDark,
                       180.0f, 0.0f, 0.0f);
}

// ---------------------------------------------------------------------------
// Hierarchical Component: Seats (Thwarts)
// ---------------------------------------------------------------------------
void Boat::drawSeats(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const
{
    const glm::vec3 seatWood(0.48f, 0.34f, 0.22f); // Lighter sanded bench wood
    const glm::vec3 riserWood(0.30f, 0.20f, 0.12f);

    // 1. Center Rowing Thwart (Seat where the rower sits)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.14f, 0.05f),
             glm::vec3(1.02f, 0.06f, 0.36f),
             seatWood);

    // Center seat support knees/risers
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.07f, 0.05f),
             glm::vec3(0.12f, 0.12f, 0.30f),
             riserWood);

    // 2. Stern Bench (Rear passenger seat)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.18f, -0.92f),
             glm::vec3(0.96f, 0.06f, 0.38f),
             seatWood);

    // 3. Bow Step / Forward Thwart
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.20f, 0.95f),
             glm::vec3(0.82f, 0.06f, 0.30f),
             seatWood);
}

// ---------------------------------------------------------------------------
// Hierarchical Component: Oars (Rowlocks, Oar Shafts, Flat Blades)
// ---------------------------------------------------------------------------
void Boat::drawOars(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder, const glm::mat4& rootMat) const
{
    const glm::vec3 ironOarlock(0.16f, 0.16f, 0.18f); // Cast iron oarlock
    const glm::vec3 oarWood(0.55f, 0.40f, 0.25f);     // Ash/spruce oar shaft
    const glm::vec3 bladeWood(0.52f, 0.36f, 0.22f);   // Flat paddle blade
    const glm::vec3 oarHandle(0.28f, 0.18f, 0.12f);   // Handle grip

    // ------------------------------------------------------------------
    // Port (Left) Oar: Pivoting around left gunwale oarlock (-0.58, 0.40, 0.10)
    // ------------------------------------------------------------------
    glm::vec3 portPivot(-0.58f, 0.40f, 0.10f);

    // Left Oarlock base
    drawPart(shader, unitCube, rootMat,
             portPivot,
             glm::vec3(0.06f, 0.08f, 0.06f),
             ironOarlock);

    // Left Oar Root (Pivoted by dynamic oarSwingAngle)
    glm::mat4 portOarMat = glm::translate(rootMat, portPivot);
    portOarMat = glm::rotate(portOarMat, glm::radians(oarSwingAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Row sweep
    portOarMat = glm::rotate(portOarMat, glm::radians(-14.0f), glm::vec3(0.0f, 0.0f, 1.0f));        // Dip into water
    portOarMat = glm::rotate(portOarMat, glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Inboard Handle (Extending toward boat center: +X)
    drawCylinderPart(shader, unitCylinder, portOarMat,
                     glm::vec3(0.25f, 0.0f, 0.0f),
                     glm::vec3(0.032f, 0.50f, 0.032f),
                     oarHandle,
                     0.0f, 0.0f, 90.0f);

    // Outboard Shaft (Extending into water: -X)
    drawCylinderPart(shader, unitCylinder, portOarMat,
                     glm::vec3(-0.65f, 0.0f, 0.0f),
                     glm::vec3(0.035f, 1.30f, 0.035f),
                     oarWood,
                     0.0f, 0.0f, 90.0f);

    // Flat Oar Blade at tip
    drawPart(shader, unitCube, portOarMat,
             glm::vec3(-1.42f, 0.0f, 0.0f),
             glm::vec3(0.48f, 0.015f, 0.16f),
             bladeWood,
             0.0f, 0.0f, 0.0f);

    // ------------------------------------------------------------------
    // Starboard (Right) Oar: Pivoting around right gunwale oarlock (+0.58, 0.40, 0.10)
    // ------------------------------------------------------------------
    glm::vec3 stbdPivot(0.58f, 0.40f, 0.10f);

    // Right Oarlock base
    drawPart(shader, unitCube, rootMat,
             stbdPivot,
             glm::vec3(0.06f, 0.08f, 0.06f),
             ironOarlock);

    // Right Oar Root (Pivoted opposite to mirror rowing motion)
    glm::mat4 stbdOarMat = glm::translate(rootMat, stbdPivot);
    stbdOarMat = glm::rotate(stbdOarMat, glm::radians(-oarSwingAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Row sweep
    stbdOarMat = glm::rotate(stbdOarMat, glm::radians(14.0f), glm::vec3(0.0f, 0.0f, 1.0f));          // Dip into water
    stbdOarMat = glm::rotate(stbdOarMat, glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Inboard Handle (Extending toward boat center: -X)
    drawCylinderPart(shader, unitCylinder, stbdOarMat,
                     glm::vec3(-0.25f, 0.0f, 0.0f),
                     glm::vec3(0.032f, 0.50f, 0.032f),
                     oarHandle,
                     0.0f, 0.0f, 90.0f);

    // Outboard Shaft (Extending into water: +X)
    drawCylinderPart(shader, unitCylinder, stbdOarMat,
                     glm::vec3(0.65f, 0.0f, 0.0f),
                     glm::vec3(0.035f, 1.30f, 0.035f),
                     oarWood,
                     0.0f, 0.0f, 90.0f);

    // Flat Oar Blade at tip
    drawPart(shader, unitCube, stbdOarMat,
             glm::vec3(1.42f, 0.0f, 0.0f),
             glm::vec3(0.48f, 0.015f, 0.16f),
             bladeWood,
             0.0f, 0.0f, 0.0f);
}

// ---------------------------------------------------------------------------
// Hierarchical Component: Details
// (Interior ribs, bow mooring ring, small brass lantern)
// ---------------------------------------------------------------------------
void Boat::drawDetails(const Shader& shader, const Cube& unitCube, const glm::mat4& rootMat) const
{
    const glm::vec3 ribWood(0.26f, 0.18f, 0.12f);
    const glm::vec3 mooringMetal(0.20f, 0.20f, 0.22f);
    const glm::vec3 lanternBrass(0.35f, 0.28f, 0.14f);
    const glm::vec3 lanternGlow(1.0f, 0.76f, 0.40f);

    // 1. Transverse Floor Ribs (Reinforcing frames across interior floor)
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.04f, -0.45f),
             glm::vec3(0.96f, 0.04f, 0.06f),
             ribWood);

    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.04f, 0.55f),
             glm::vec3(0.90f, 0.04f, 0.06f),
             ribWood);

    // 2. Bow Mooring Eyelet & Cleat
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.38f, 1.55f),
             glm::vec3(0.05f, 0.06f, 0.12f),
             mooringMetal);

    // 3. Small Mooring Lantern sitting on the bow plate
    // Lantern base
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.38f, 1.25f),
             glm::vec3(0.12f, 0.04f, 0.12f),
             lanternBrass);

    // Glowing lantern glass chamber
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.46f, 1.25f),
             glm::vec3(0.09f, 0.12f, 0.09f),
             glm::vec3(0.95f, 0.80f, 0.50f),
             0.0f, 0.0f, 0.0f,
             true, lanternGlow * 1.5f);

    // Lantern top cap
    drawPart(shader, unitCube, rootMat,
             glm::vec3(0.0f, 0.54f, 1.25f),
             glm::vec3(0.13f, 0.03f, 0.13f),
             lanternBrass);
}

// ---------------------------------------------------------------------------
// Main Render Pass
// ---------------------------------------------------------------------------
void Boat::draw(const Shader& shader,
                const Cube& unitCube,
                const TriangularPrism& unitPrism,
                const Cylinder& unitCylinder) const
{
    glm::mat4 rootMat = getRootMatrix();

    // 1. Hull
    drawHull(shader, unitCube, unitPrism, rootMat);

    // 2. Seats
    drawSeats(shader, unitCube, rootMat);

    // 3. Oars
    drawOars(shader, unitCube, unitCylinder, rootMat);

    // 4. Details
    drawDetails(shader, unitCube, rootMat);
}

void Boat::draw(const Shader& shader,
                const Cube& unitCube,
                const TriangularPrism& unitPrism) const
{
    Cylinder tempCylinder(16);
    draw(shader, unitCube, unitPrism, tempCylinder);
}
