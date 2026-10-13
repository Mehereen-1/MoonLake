#ifndef BRANCH_H
#define BRANCH_H

#include "Mesh.h"

class Branch : public Mesh
{
public:
    // Low-poly fallen wooden branch
    Branch(float length = 1.8f, float radius = 0.10f);
    virtual ~Branch() = default;
};

#endif // BRANCH_H
