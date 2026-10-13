#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>

Mesh::Mesh()
    : VAO(0), VBO(0), EBO(0),
      vertexCount(0), indexCount(0), hasIndices(false),
      position(0.0f),
      rotationAngleDegrees(0.0f),
      rotationAxis(0.0f, 1.0f, 0.0f),
      scale(1.0f)
{
}

Mesh::~Mesh()
{
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
}

void Mesh::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Mesh::setRotation(float angleDegrees, const glm::vec3& axis)
{
    rotationAngleDegrees = angleDegrees;
    rotationAxis = axis;
}

void Mesh::setScale(const glm::vec3& scl)
{
    scale = scl;
}

const glm::vec3& Mesh::getPosition() const
{
    return position;
}

float Mesh::getRotationAngle() const
{
    return rotationAngleDegrees;
}

const glm::vec3& Mesh::getRotationAxis() const
{
    return rotationAxis;
}

const glm::vec3& Mesh::getScale() const
{
    return scale;
}

glm::mat4 Mesh::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    if (rotationAngleDegrees != 0.0f && glm::length(rotationAxis) > 0.0001f)
    {
        model = glm::rotate(model, glm::radians(rotationAngleDegrees), glm::normalize(rotationAxis));
    }

    model = glm::scale(model, scale);
    return model;
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    vertexCount = vertices.size();
    indexCount = indices.size();
    hasIndices = !indices.empty();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    if (hasIndices)
    {
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    if (hasIndices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    // 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // 1: Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // 2: Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::setupMeshRaw(const float* vertexData, size_t sizeInBytes, size_t count)
{
    vertexCount = count;
    hasIndices = false;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertexData, GL_STATIC_DRAW);

    // Stride is 9 floats: [Position (3), Normal (3), Color (3)]
    size_t stride = 9 * sizeof(float);

    // 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)0);
    glEnableVertexAttribArray(0);

    // 1: Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 2: Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const
{
    glBindVertexArray(VAO);
    if (hasIndices)
    {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    }
    glBindVertexArray(0);
}
