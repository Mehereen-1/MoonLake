#ifndef MOUNTAIN_H
#define MOUNTAIN_H

#include "Mesh.h"

class Mountain : public Mesh
{
public:
    // Generates a low-poly jagged alpine mountain ridge
    Mountain(float width = 60.0f, float height = 18.0f, float depth = 16.0f);
    virtual ~Mountain() = default;
};

#endif // MOUNTAIN_H
