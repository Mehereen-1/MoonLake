#ifndef WATER_H
#define WATER_H

#include "Mesh.h"

class Water : public Mesh
{
public:
    // Subdivided water surface grid for vertex wave displacement
    Water(float width = 45.0f, float depth = 40.0f, int gridX = 40, int gridZ = 40);
    virtual ~Water() = default;
};

#endif // WATER_H
