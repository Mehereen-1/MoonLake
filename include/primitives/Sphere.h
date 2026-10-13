#ifndef SPHERE_H
#define SPHERE_H

#include "Mesh.h"

class Sphere : public Mesh
{
public:
    Sphere(float radius = 1.0f,
           unsigned int sectorCount = 36,
           unsigned int stackCount = 18,
           const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.95f));
    virtual ~Sphere() = default;
};

#endif // SPHERE_H
