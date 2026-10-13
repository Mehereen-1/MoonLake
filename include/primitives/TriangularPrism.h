#ifndef TRIANGULAR_PRISM_H
#define TRIANGULAR_PRISM_H

#include "Mesh.h"

class TriangularPrism : public Mesh
{
public:
    // Generates a canonical unit triangular prism for roofs, wedges, and ramps:
    // Base: X in [-0.5, 0.5], Z in [-0.5, 0.5] at Y = 0.0
    // Apex ridge line: X = 0.0, Z in [-0.5, 0.5] at Y = 1.0
    // Total 5 faces (8 triangles, 24 vertices with flat surface normals)
    TriangularPrism();
    virtual ~TriangularPrism() = default;
};

#endif // TRIANGULAR_PRISM_H
