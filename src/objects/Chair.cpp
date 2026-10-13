#include "objects/Chair.h"
#include <glm/gtc/matrix_transform.hpp>

Chair::Chair(const glm::vec3& pos, float rotY, float scale)
    : position(pos), rotationY(rotY), scaleFactor(scale)
{
}

void Chair::setPosition(const glm::vec3& pos) { position = pos; }
void Chair::setRotation(float rotY)           { rotationY = rotY; }
void Chair::setScale(float scale)             { scaleFactor = scale; }

const glm::vec3& Chair::getPosition() const   { return position; }
float Chair::getRotation() const              { return rotationY; }
float Chair::getScale() const                 { return scaleFactor; }

void Chair::drawPart(const Shader& shader, const Cube& unitCube, const glm::mat4& parentMat,
                     const glm::vec3& offset, const glm::vec3& scale, const glm::vec3& color,
                     float rotY, float rotX, float rotZ) const
{
    glm::mat4 model = parentMat;
    model = glm::translate(model, offset);
    if (rotY != 0.0f) model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotX != 0.0f) model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotZ != 0.0f) model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setBool("isEmissive", false);

    unitCube.draw();
}

void Chair::draw(const Shader& shader, const Cube& unitCube) const
{
    glm::mat4 rootMat(1.0f);
    rootMat = glm::translate(rootMat, position);
    rootMat = glm::rotate(rootMat, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    rootMat = glm::scale(rootMat, glm::vec3(scaleFactor));

    const glm::vec3 woodMain(0.38f, 0.25f, 0.16f); // Weathered mountain cedar
    const glm::vec3 woodDark(0.26f, 0.16f, 0.10f); // Shadow frame

    // 1. Front Legs
    drawPart(shader, unitCube, rootMat, glm::vec3(-0.35f, 0.28f,  0.30f), glm::vec3(0.08f, 0.56f, 0.08f), woodDark);
    drawPart(shader, unitCube, rootMat, glm::vec3( 0.35f, 0.28f,  0.30f), glm::vec3(0.08f, 0.56f, 0.08f), woodDark);

    // 2. Angled Side Rail Beams (Sloping downwards to the rear)
    drawPart(shader, unitCube, rootMat, glm::vec3(-0.32f, 0.26f, -0.05f), glm::vec3(0.06f, 0.10f, 0.85f), woodDark, 0.0f, 14.0f, 0.0f);
    drawPart(shader, unitCube, rootMat, glm::vec3( 0.32f, 0.26f, -0.05f), glm::vec3(0.06f, 0.10f, 0.85f), woodDark, 0.0f, 14.0f, 0.0f);

    // 3. Slanted Seat Slats (4 horizontal timber slats)
    for (int i = 0; i < 4; ++i)
    {
        float z = 0.26f - i * 0.16f;
        float y = 0.38f - i * 0.06f;
        drawPart(shader, unitCube, rootMat, glm::vec3(0.0f, y, z), glm::vec3(0.68f, 0.04f, 0.13f), woodMain, 0.0f, 14.0f, 0.0f);
    }

    // 4. Wide Flat Armrests (Adirondack signature style)
    drawPart(shader, unitCube, rootMat, glm::vec3(-0.42f, 0.54f, 0.05f), glm::vec3(0.18f, 0.04f, 0.78f), woodMain);
    drawPart(shader, unitCube, rootMat, glm::vec3( 0.42f, 0.54f, 0.05f), glm::vec3(0.18f, 0.04f, 0.78f), woodMain);

    // 5. High Angled Backrest (5 vertical wooden slats tilted backwards ~ 22 deg)
    for (int i = -2; i <= 2; ++i)
    {
        float x = i * 0.12f;
        float h = 0.75f - std::abs(i) * 0.06f; // Rounded fan top
        drawPart(shader, unitCube, rootMat, glm::vec3(x, 0.65f, -0.36f), glm::vec3(0.10f, h, 0.035f), woodMain, 0.0f, -22.0f, 0.0f);
    }

    // Rear Support Braces
    drawPart(shader, unitCube, rootMat, glm::vec3(0.0f, 0.18f, -0.42f), glm::vec3(0.60f, 0.06f, 0.06f), woodDark);
}
