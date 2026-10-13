#include "primitives/GlowQuad.h"

GlowQuad::GlowQuad(float size)
{
    float h = size * 0.5f;

    // Normal.z = 1.0 (quad face normal), vertexColor.xy contains normalized [-1, 1] coordinates
    std::vector<Vertex> vertices = {
        { glm::vec3(-h, -h, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, -1.0f, 0.0f) },
        { glm::vec3( h, -h, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3( 1.0f, -1.0f, 0.0f) },
        { glm::vec3( h,  h, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3( 1.0f,  1.0f, 0.0f) },
        { glm::vec3(-h,  h, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f,  1.0f, 0.0f) }
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
    setupMesh(vertices, indices);
}
