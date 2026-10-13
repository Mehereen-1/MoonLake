#ifndef TERRAIN_MESH_H
#define TERRAIN_MESH_H

#include "Mesh.h"
#include <glm/glm.hpp>

class TerrainMesh : public Mesh
{
public:
    // Generates a continuous, high-resolution organic terrain grid covering the entire world
    // with smooth slopes, natural lake basin, and surrounding hills/bluffs.
    TerrainMesh(float minX = -85.0f, float maxX = 85.0f,
                float minZ = -60.0f, float maxZ = 130.0f,
                int gridX = 170, int gridZ = 190);

    // Overload for backward compatibility
    TerrainMesh(bool isWestSide);
    virtual ~TerrainMesh() = default;

    // Shoreline and elevation functions shared across terrain, grass, and props
    static float getShorelineX(float z);
    static float getEastShorelineX(float z);
    static float getNorthShorelineZ(float x);
    static float getSouthShorelineZ(float x);
    static float computeElevation(float x, float z, bool& isValidLand);
};

#endif // TERRAIN_MESH_H

