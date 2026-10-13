#ifndef AUTUMN_TREE_H
#define AUTUMN_TREE_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Shader.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"
#include "primitives/Pyramid.h"
#include "objects/StylizedPineMesh.h"

// ---------------------------------------------------------------------------
// AutumnTree Class (Stylized Autumn Pine Tree with Warm Glowing Foliage)
// ---------------------------------------------------------------------------
class AutumnTree
{
public:
    AutumnTree(const glm::vec3& pos = glm::vec3(0.0f), float scale = 1.0f, float rotY = 0.0f, int colorPalette = 0);

    void setPosition(const glm::vec3& pos);
    void setScale(float scale);
    void setRotation(float rotY);

    const glm::vec3& getPosition() const;
    float getScale() const;
    float getRotation() const;

    void draw(const Shader& shader, const Cylinder& unitCylinder, const Sphere& unitSphere, const Pyramid& unitPyramid) const;
    void draw(const Shader& shader) const;

private:
    glm::vec3 position;
    float scaleFactor;
    float rotationY;
    int paletteType; // 0 = Amber Orange, 1 = Golden Yellow, 2 = Russet Crimson, 3 = Warm Ochre
    int variantIndex;

    glm::mat4 getRootMatrix() const;

    static void ensureMeshes();
    // 5 variants x 4 autumn palettes = 20 cached meshes
    static std::vector<std::unique_ptr<StylizedPineMesh>> sharedVariants;
    static bool meshesInitialized;
};

#endif // AUTUMN_TREE_H
