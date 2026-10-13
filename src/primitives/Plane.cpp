#include "primitives/Plane.h"

Plane::Plane(float width, float depth, const glm::vec3& color, bool vertical)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    if (vertical)
    {
        // Vertical plane along XY (facing -Z towards camera)
        glm::vec3 normal(0.0f, 0.0f, 1.0f);
        vertices = {
            { glm::vec3(-hw, -hd, 0.0f), normal, color },
            { glm::vec3( hw, -hd, 0.0f), normal, color },
            { glm::vec3( hw,  hd, 0.0f), normal, color },
            { glm::vec3(-hw,  hd, 0.0f), normal, color }
        };
    }
    else
    {
        // Horizontal plane along XZ (facing +Y upwards)
        glm::vec3 normal(0.0f, 1.0f, 0.0f);
        vertices = {
            { glm::vec3(-hw, 0.0f, -hd), normal, color },
            { glm::vec3( hw, 0.0f, -hd), normal, color },
            { glm::vec3( hw, 0.0f,  hd), normal, color },
            { glm::vec3(-hw, 0.0f,  hd), normal, color }
        };
    }

    indices = { 0, 1, 2, 0, 2, 3 };
    setupMesh(vertices, indices);
}
