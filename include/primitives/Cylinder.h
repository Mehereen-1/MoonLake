#ifndef CYLINDER_H
#define CYLINDER_H

#include "Mesh.h"

class Cylinder : public Mesh
{
public:
    // Generates a canonical unit cylinder:
    // Radius = 0.5 (diameter 1.0), Height = 1.0 (from Y = 0.0 to Y = 1.0)
    // Centered at (0, 0) in X-Z, with top/bottom disc caps and outward radial normals
    Cylinder(int segments = 16);
    virtual ~Cylinder() = default;
};

#endif // CYLINDER_H
