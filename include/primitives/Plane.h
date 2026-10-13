#ifndef PLANE_H
#define PLANE_H

#include "Mesh.h"

class Plane : public Mesh
{
public:
    Plane(float width = 20.0f,
          float depth = 20.0f,
          const glm::vec3& color = glm::vec3(0.08f, 0.16f, 0.10f),
          bool vertical = false);
    virtual ~Plane() = default;
};

#endif // PLANE_H
