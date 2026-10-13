#ifndef GRASS_TUFT_H
#define GRASS_TUFT_H

#include "Mesh.h"

class GrassTuft : public Mesh
{
public:
    // Low-poly crossed blade grass tuft
    GrassTuft(float scale = 1.0f);
    virtual ~GrassTuft() = default;
};

#endif // GRASS_TUFT_H
