#ifndef CAVE_H
#define CAVE_H

#include "Shader.h"
#include "Mesh.h"
#include "primitives/Cube.h"
#include "primitives/TriangularPrism.h"
#include "primitives/Pyramid.h"
#include "primitives/Sphere.h"
#include "primitives/Plane.h"
#include "primitives/Rock.h"
#include "primitives/Cylinder.h"
#include <glm/glm.hpp>
#include <vector>

// Forward declaration of the motes state structure defined in MotionSystem.h
struct CaveMotesState;

// Simple pathway section definition
struct CavePathSection {
    glm::vec3 center;    // Centerline waypoint Pi
    float width;         // Corridor width Wi
    float height;        // Ceiling height Hi
    float length;        // Section depth along Z Li
    float wallThick;     // Wall thickness
};

// Derived Mesh subclass to expose buffer setup for the procedural cobblestone tunnel
class CobblestoneTunnelMesh : public Mesh
{
public:
    void init(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    {
        setupMesh(vertices, indices);
    }
};

class Cave
{
public:
    Cave(const glm::vec3& pos = glm::vec3(0.0f), float rotY = 0.0f, float scale = 1.0f);

    void setPosition(const glm::vec3& pos);
    void setRotation(float rotY);
    void setScale(float scale);

    const glm::vec3& getPosition() const;
    float getRotation() const;
    float getScale() const;

    // Root world transformation matrix
    glm::mat4 getRootMatrix() const;

    // World-space position of the warm light inside the tunnel
    glm::vec3 getInteriorLightPos() const;

    // Mathematical Cave Pathway Definition (P0 -> P1 -> P2 -> P3)
    std::vector<CavePathSection> getPathSections() const;

    // Guided Cave Tour Camera Path Evaluation:
    // tourProgress in [0.0, 3.0] smoothly interpolates through P0 -> P1 -> P2 -> P3
    glm::vec3 getTourWorldPosition(float tourProgress) const;
    glm::vec3 getTourWorldLookTarget(float tourProgress) const;

    // ------------------------------------------------------------------
    // Localized Glowing Treasure Hoard & Gold Coins System
    // (Area-only illumination: illuminates only the immediate surrounding area)
    // ------------------------------------------------------------------
    glm::vec3 getTreasureWorldPos() const;
    glm::vec3 getTreasureLightColor(float time = 0.0f) const;
    float getTreasureLightIntensity(float time = 0.0f) const;
    glm::vec3 getTreasureViewCameraPos() const;
    glm::vec3 getTreasureViewLookTarget() const;

    // ------------------------------------------------------------------
    // Real-Time Ray-Traced Gold Coins & Treasure Hoard System
    // ------------------------------------------------------------------
    bool isCoinRayTracingEnabled() const { return m_coinRayTracingEnabled; }
    void toggleCoinRayTracing() { m_coinRayTracingEnabled = !m_coinRayTracingEnabled; }
    void setCoinRayTracingEnabled(bool enabled) { m_coinRayTracingEnabled = enabled; }
    glm::vec3 getHangingOrbWorldPos() const;
    glm::vec3 getChestMinBounds() const;
    glm::vec3 getChestMaxBounds() const;

    // ------------------------------------------------------------------
    // Controllable Player Flaming Torch System (Outside entrance by default; taken on demand)
    // ------------------------------------------------------------------
    bool isTorchLit() const { return torchLit; }
    void toggleTorchLit();
    void setTorchLit(bool lit) { torchLit = lit; }

    bool isPlayerCarryingTorch() const { return playerHasTorch; }
    void setPlayerCarryingTorch(bool carrying) { playerHasTorch = carrying; }
    void togglePlayerTorch() { playerHasTorch = !playerHasTorch; }

    glm::vec3 getTorchStandWorldPos() const;
    glm::vec3 getActiveTorchLightPos(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camRight, const glm::vec3& camUp) const;

    float getTorchBrightness() const { return torchBrightness; }
    const char* getTorchBrightnessName() const;
    void cycleTorchBrightness();
    void setTorchBrightness(float b) { torchBrightness = b; }

    // Draw first-person handheld torch carried & controlled by the user
    void drawHandheldTorch(const Shader& shader,
                           const Cube& unitCube,
                           const Cylinder& unitCylinder,
                           const Sphere& unitSphere,
                           const Pyramid& unitPyramid,
                           const glm::vec3& camPos,
                           const glm::vec3& camFront,
                           const glm::vec3& camRight,
                           const glm::vec3& camUp,
                           float time) const;

    // Primary draw pass receiving all unit primitives and precalculated motes
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Plane& unitPlane,
              const Cylinder& unitCylinder,
              const Rock& unitRock,
              const CaveMotesState& motesState,
              float frameTime = 0.0f) const;

    // Backward compatibility overloads
    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Plane& unitPlane,
              const Rock& unitRock,
              const CaveMotesState& motesState) const;

    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Plane& unitPlane,
              const Rock& unitRock,
              const CaveMotesState& motesState) const;

    void draw(const Shader& shader,
              const Cube& unitCube,
              const TriangularPrism& unitPrism,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Plane& unitPlane,
              const Rock& unitRock) const;

    void draw(const Shader& shader,
              const Cube& unitCube,
              const Pyramid& unitPyramid,
              const Sphere& unitSphere,
              const Plane& unitPlane,
              const Rock& unitRock) const;

private:
    glm::vec3 position;
    float rotationY;
    float scaleFactor;

    // Controllable Flaming Torch State (Outside entrance by default, taken on demand)
    bool playerHasTorch = false; // Starts outside on stand; user takes it when they want
    bool torchLit = true;
    float torchBrightness = 1.0f;
    int brightnessLevel = 1; // 0 = Dim (0.5x), 1 = Normal (1.0x), 2 = Bright (1.6x), 3 = Blazing (2.2x)
    bool m_coinRayTracingEnabled = true;

    // Procedural stone masonry tunnel mesh (cobblestone walls, arch, mortar, floor, and outer stone vault)
    CobblestoneTunnelMesh tunnelMesh;

    // Procedural geometry generator matching the uploaded cobblestone tunnel reference image
    void buildCobblestoneTunnel();

    // Drawing helpers using unit primitives
    void drawCube(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                  const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                  float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                  bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawPlane(const Shader& shader, const Plane& unitPlane, const glm::mat4& parentMat,
                   const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                   float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                   bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                    const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                    bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawSphere(const Shader& shader, const Sphere& unitSphere, const glm::mat4& parentMat,
                    const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                    float rotY, float rotX, float rotZ,
                    bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawCylinder(const Shader& shader, const Cylinder& unitCylinder, const glm::mat4& parentMat,
                      const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                      float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                      bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    void drawPyramid(const Shader& shader, const Pyramid& unitPyramid, const glm::mat4& parentMat,
                     const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                     float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f,
                     bool emissive = false, const glm::vec3& emitColor = glm::vec3(1.0f)) const;

    // Handheld torch drawing helper
    void drawTorchMesh(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                       const Sphere& unitSphere, const Pyramid& unitPyramid,
                       const glm::mat4& baseMat, float time) const;

    // Torch stand outside the cave entrance
    void drawTorchStand(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                        const Sphere& unitSphere, const Pyramid& unitPyramid,
                        const glm::mat4& rootMat, float time) const;

    // Soft dim light portal at the end of the tunnel (Deprecated stub: replaced by solid stone back wall)
    void drawDimLightAtEnd(const Shader& shader, const Plane& unitPlane, const Cube& unitCube,
                           const Sphere& unitSphere, const glm::mat4& rootMat) const;

    // Floating atmospheric dust specks (Deprecated stub: tunnel is naturally dark, lit solely by player torch)
    void drawFloatingMotes(const Shader& shader, const Sphere& unitSphere,
                           const glm::mat4& rootMat, const CaveMotesState& motesState) const;

    // Detailed open treasure chest and illuminating gold coins (with cylinders)
    void drawTreasureChestAndCoins(const Shader& shader,
                                  const Cube& unitCube,
                                  const Cylinder& unitCylinder,
                                  const Sphere& unitSphere,
                                  const Pyramid& unitPyramid,
                                  const glm::mat4& rootMat,
                                  float time) const;

    void drawCoin(const Shader& shader,
                  const Cylinder& unitCylinder,
                  const glm::mat4& parentMat,
                  const glm::vec3& offset,
                  const glm::vec3& scale,
                  const glm::vec3& color,
                  float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) const;

    // Atmospheric Fire and Torch Elements for the Treasure Sanctum
    void drawWallTorchSconce(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                             const Sphere& unitSphere, const Pyramid& unitPyramid,
                             const glm::mat4& rootMat, const glm::vec3& pos, float wallSide,
                             float time, float phase) const;

    void drawStandingBrazier(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                             const Sphere& unitSphere, const Pyramid& unitPyramid,
                             const glm::mat4& rootMat, const glm::vec3& pos,
                             float time, float phase) const;

    void drawClosedChest(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder,
                         const Sphere& unitSphere, const Pyramid& unitPyramid,
                         const glm::mat4& rootMat, const glm::vec3& pos, float rotY, float scale) const;
};

#endif // CAVE_H
