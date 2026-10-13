#ifndef GLOW_QUAD_H
#define GLOW_QUAD_H

#include "Mesh.h"

class GlowQuad : public Mesh
{
public:
    GlowQuad(float size = 1.0f);
    virtual ~GlowQuad() = default;
};

#endif // GLOW_QUAD_H
