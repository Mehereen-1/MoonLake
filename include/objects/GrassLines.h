#ifndef GRASS_LINES_H
#define GRASS_LINES_H

#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// GrassLines
// Renders thousands of slender, organic grass blades across the entire terrain
// using lightweight GL_LINES primitives with smooth base-to-tip color gradients.
// Replaces chunky 3D pyramid meshes with delicate, natural grass lines.
// ---------------------------------------------------------------------------
class GrassLines : public Mesh
{
public:
    // Generates the line grass field across the campsite, shoreline, and eastern shores
    GrassLines(float defaultHeight = 0.26f, float lineWidth = 1.8f);
    virtual ~GrassLines() = default;

    // Custom draw using GL_LINES
    void draw() const override;

    float getLineWidth() const { return m_lineWidth; }
    void setLineWidth(float width) { m_lineWidth = width; }

private:
    float m_lineWidth;

    // Evaluates terrain elevation Y at world position (X, Z)
    static float getGroundElevation(float x, float z, bool& isValidGround);

    // Calculates the sinuous lake waterline X coordinate for a given Z
    static float getShorelineX(float z);
};

#endif // GRASS_LINES_H
