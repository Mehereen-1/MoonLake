#ifndef TREELINE_H
#define TREELINE_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Pyramid.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"

// ---------------------------------------------------------------------------
// Treeline
// Manages a dense, multi-tiered panoramic forest across the southern distant ridge
// (Z = 50m to 105m, X = -65m to +65m), ensuring that looking from the lake,
// boat, or campsite, the horizon is completely crowned with majestic natural trees
// and the boundary of the world is entirely obscured.
// ---------------------------------------------------------------------------
class Treeline
{
public:
    struct DistantTree {
        glm::vec3 pos;
        float scale;
        float rotY;
        int treeType; // 0 = Tall Alpine Pine, 1 = Dense Forest Spruce, 2 = Broadleaf Canopy
        glm::vec3 foliageColor;
    };

    Treeline();

    void draw(const Shader& shader,
              const Cube& unitCube,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              const Pyramid& unitPyramid) const;

private:
    std::vector<DistantTree> trees;
    void initForest();
};

#endif // TREELINE_H
