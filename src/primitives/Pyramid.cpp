#include "primitives/Pyramid.h"

Pyramid::Pyramid()
{
    // Stride: Position (X,Y,Z), Normal (NX,NY,NZ), Color (R,G,B)
    static const float vertices[] = {
        // Base triangle 1 (Normal: 0, -1, 0)
        -0.5f, 0.0f, -0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,
         0.5f, 0.0f, -0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,
         0.5f, 0.0f,  0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,

        // Base triangle 2 (Normal: 0, -1, 0)
         0.5f, 0.0f,  0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,
        -0.5f, 0.0f,  0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,
        -0.5f, 0.0f, -0.5f,   0.0f, -1.0f,  0.0f,   0.15f, 0.70f, 0.80f,

        // Front face (Normal ~ 0, 0.53, 0.85)
        -0.5f, 0.0f,  0.5f,   0.0f,  0.53f,  0.85f,  0.95f, 0.45f, 0.10f,
         0.5f, 0.0f,  0.5f,   0.0f,  0.53f,  0.85f,  0.95f, 0.45f, 0.10f,
         0.0f, 0.8f,  0.0f,   0.0f,  0.53f,  0.85f,  1.00f, 0.90f, 0.20f,

        // Right face (Normal ~ 0.85, 0.53, 0)
         0.5f, 0.0f,  0.5f,   0.85f, 0.53f,  0.0f,   0.90f, 0.20f, 0.60f,
         0.5f, 0.0f, -0.5f,   0.85f, 0.53f,  0.0f,   0.90f, 0.20f, 0.60f,
         0.0f, 0.8f,  0.0f,   0.85f, 0.53f,  0.0f,   1.00f, 0.90f, 0.20f,

        // Back face (Normal ~ 0, 0.53, -0.85)
         0.5f, 0.0f, -0.5f,   0.0f,  0.53f, -0.85f,  0.20f, 0.85f, 0.40f,
        -0.5f, 0.0f, -0.5f,   0.0f,  0.53f, -0.85f,  0.20f, 0.85f, 0.40f,
         0.0f, 0.8f,  0.0f,   0.0f,  0.53f, -0.85f,  1.00f, 0.90f, 0.20f,

        // Left face (Normal ~ -0.85, 0.53, 0)
        -0.5f, 0.0f, -0.5f,  -0.85f, 0.53f,  0.0f,   0.20f, 0.50f, 0.95f,
        -0.5f, 0.0f,  0.5f,  -0.85f, 0.53f,  0.0f,   0.20f, 0.50f, 0.95f,
         0.0f, 0.8f,  0.0f,  -0.85f, 0.53f,  0.0f,   1.00f, 0.90f, 0.20f
    };

    setupMeshRaw(vertices, sizeof(vertices), 18);
}
