#include "objects/AutumnTree.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

std::vector<std::unique_ptr<StylizedPineMesh>> AutumnTree::sharedVariants;
bool AutumnTree::meshesInitialized = false;

AutumnTree::AutumnTree(const glm::vec3& pos, float scale, float rotY, int colorPalette)
    : position(pos), scaleFactor(scale), rotationY(rotY), paletteType(colorPalette)
{
    variantIndex = std::abs(static_cast<int>(std::floor(pos.x * 5.17f + pos.z * 11.23f))) % 5;
}

void AutumnTree::setPosition(const glm::vec3& pos) { position = pos; }
void AutumnTree::setScale(float scale)             { scaleFactor = scale; }
void AutumnTree::setRotation(float rotY)           { rotationY = rotY; }

const glm::vec3& AutumnTree::getPosition() const   { return position; }
float AutumnTree::getScale() const                 { return scaleFactor; }
float AutumnTree::getRotation() const              { return rotationY; }

glm::mat4 AutumnTree::getRootMatrix() const
{
    glm::mat4 root(1.0f);
    root = glm::translate(root, position);
    root = glm::rotate(root, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::scale(root, glm::vec3(scaleFactor));
    return root;
}

void AutumnTree::ensureMeshes()
{
    if (!meshesInitialized)
    {
        sharedVariants.reserve(20);
        // 4 autumn palettes (1=Amber, 2=Golden, 3=Crimson, 4=Copper) x 5 procedural variants
        for (int p = 0; p < 4; ++p)
        {
            for (int v = 0; v < 5; ++v)
            {
                sharedVariants.push_back(std::make_unique<StylizedPineMesh>(v, p + 1));
            }
        }
        meshesInitialized = true;
    }
}

void AutumnTree::draw(const Shader& shader, const Cylinder&, const Sphere&, const Pyramid&) const
{
    draw(shader);
}

void AutumnTree::draw(const Shader& shader) const
{
    ensureMeshes();

    glm::mat4 root = getRootMatrix();

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 16.0f);
    shader.setMat4("model", root);
    shader.setVec3("objectColor", glm::vec3(0.0f)); // Use rich autumn vertex colors

    int pal = (paletteType % 4);
    int meshIdx = pal * 5 + (variantIndex % 5);

    if (meshIdx >= 0 && meshIdx < static_cast<int>(sharedVariants.size()))
    {
        sharedVariants[meshIdx]->draw(shader);
    }
}
