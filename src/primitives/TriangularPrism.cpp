#include "primitives/TriangularPrism.h"

TriangularPrism::TriangularPrism()
{
    // 24 vertices (5 faces: bottom rect [6] + front gable [3] + back gable [3] + left slope [6] + right slope [6])
    // Stride: Position (X, Y, Z), Normal (NX, NY, NZ), Color (R, G, B)
    // Left slope normal: (-2/sqrt(5), 1/sqrt(5), 0) ~ (-0.894427f, 0.447214f, 0.0f)
    // Right slope normal: (2/sqrt(5), 1/sqrt(5), 0) ~ (0.894427f, 0.447214f, 0.0f)

    static const float vertices[] = {
        // 1. Bottom Face (Normal: 0, -1, 0)
        -0.5f, 0.0f, -0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,
         0.5f, 0.0f,  0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,
         0.5f, 0.0f, -0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,

        -0.5f, 0.0f, -0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,
        -0.5f, 0.0f,  0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,
         0.5f, 0.0f,  0.5f,    0.0f, -1.0f,  0.0f,    0.20f, 0.20f, 0.20f,

        // 2. Front Gable Face (Normal: 0, 0, 1)
        -0.5f, 0.0f,  0.5f,    0.0f,  0.0f,  1.0f,    0.35f, 0.22f, 0.14f,
         0.5f, 0.0f,  0.5f,    0.0f,  0.0f,  1.0f,    0.35f, 0.22f, 0.14f,
         0.0f, 1.0f,  0.5f,    0.0f,  0.0f,  1.0f,    0.35f, 0.22f, 0.14f,

        // 3. Back Gable Face (Normal: 0, 0, -1)
         0.5f, 0.0f, -0.5f,    0.0f,  0.0f, -1.0f,    0.35f, 0.22f, 0.14f,
        -0.5f, 0.0f, -0.5f,    0.0f,  0.0f, -1.0f,    0.35f, 0.22f, 0.14f,
         0.0f, 1.0f, -0.5f,    0.0f,  0.0f, -1.0f,    0.35f, 0.22f, 0.14f,

        // 4. Left Sloped Roof Face (Normal: -0.8944f, 0.4472f, 0.0f)
        -0.5f, 0.0f, -0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f,  0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f, -0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,

        -0.5f, 0.0f, -0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
        -0.5f, 0.0f,  0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f,  0.5f,   -0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,

        // 5. Right Sloped Roof Face (Normal: 0.8944f, 0.4472f, 0.0f)
         0.5f, 0.0f, -0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f, -0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f,  0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,

         0.5f, 0.0f, -0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.0f, 1.0f,  0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f,
         0.5f, 0.0f,  0.5f,    0.8944f, 0.4472f, 0.0f,   0.28f, 0.24f, 0.22f
    };

    setupMeshRaw(vertices, sizeof(vertices), 24);
}
