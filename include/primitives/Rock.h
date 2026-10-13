#ifndef ROCK_H
#define ROCK_H

#include "Mesh.h"

class Rock : public Mesh
{
public:
    // Generates a faceted low-poly stone
    Rock(float radius = 1.0f, int seed = 0);
    virtual ~Rock() = default;
};

#endif // ROCK_H
