#ifndef WILDFLOWERS_H
#define WILDFLOWERS_H

#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>

// ---------------------------------------------------------------------------
// Wildflowers
// Generates a lush, dense carpet of blooming cosmos and meadow wildflowers
// (vibrant magenta, soft pink, lavender, lilac, white, and golden blossoms)
// matching the provided reference image. Rendered as an ultra-fast batched Mesh
// with individual vertex colors and smooth normal shading.
// ---------------------------------------------------------------------------
class Wildflowers : public Mesh
{
public:
    Wildflowers();
    virtual ~Wildflowers() = default;

    void draw(const Shader& shader) const;

private:
    void generateFlowerField();
};

#endif // WILDFLOWERS_H
