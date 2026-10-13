#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    // Transformation setters
    void setPosition(const glm::vec3& pos);
    void setRotation(float angleDegrees, const glm::vec3& axis);
    void setScale(const glm::vec3& scl);

    // Transformation getters
    const glm::vec3& getPosition() const;
    float getRotationAngle() const;
    const glm::vec3& getRotationAxis() const;
    const glm::vec3& getScale() const;

    // Calculate model transformation matrix (T * R * S)
    virtual glm::mat4 getModelMatrix() const;

    // Render the mesh
    virtual void draw() const;

protected:
    // Setup GPU buffers with Vertex structs (Pos, Normal, Color)
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {});

    // Raw float buffer setup: stride of 9 floats: [Pos(3), Normal(3), Color(3)]
    void setupMeshRaw(const float* vertexData, size_t sizeInBytes, size_t count);

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    size_t vertexCount;
    size_t indexCount;
    bool hasIndices;

    glm::vec3 position;
    float rotationAngleDegrees;
    glm::vec3 rotationAxis;
    glm::vec3 scale;
};

#endif // MESH_H
