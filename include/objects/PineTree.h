#ifndef PINETREE_H
#define PINETREE_H

#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Pyramid.h"
#include "objects/StylizedPineMesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// ---------------------------------------------------------------------------
// PineTree Class (Stylized Conifer Tree Matching Reference Image)
// Retains class name and drawing signature for 100% seamless drop-in
// compatibility across the entire MoonLake project.
// Features:
//   - Tiered whorls of stylized fanned needle boughs with downward droop
//   - Serrated needle fans with vibrant lime-chartreuse tips and emerald midtones
//   - Authentic dead lower twigs below the lowest foliage tier
//   - Fluted tapered bark trunk with root flare
//   - 5 procedural variants directly matching the 5 trees in reference image
//   - Zero-overhead static shared mesh pooling (fast draw calls)
// ---------------------------------------------------------------------------
class PineTree
{
public:
    PineTree(const glm::vec3& pos = glm::vec3(0.0f), float scale = 1.0f, float rotY = 0.0f, int variant = -1);

    void setPosition(const glm::vec3& pos);
    void setScale(float scale);
    void setRotation(float rotY);

    const glm::vec3& getPosition() const;
    float getScale() const;
    float getRotation() const;

    // Backward-compatible draw matching existing call sites in main.cpp
    void draw(const Shader& shader, const Cube& unitCube, const Pyramid& unitPyramid) const;
    void draw(const Shader& shader) const;

    int getVariantIndex() const { return variantIndex; }

private:
    glm::vec3 position;
    float scaleFactor;
    float rotationY;
    int variantIndex;

    static void ensureMeshes();
    static std::vector<std::unique_ptr<StylizedPineMesh>> sharedVariants;
    static bool meshesInitialized;
};

#endif // PINETREE_H
