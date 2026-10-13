#include "objects/PineTree.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

std::vector<std::unique_ptr<StylizedPineMesh>> PineTree::sharedVariants;
bool PineTree::meshesInitialized = false;

PineTree::PineTree(const glm::vec3& pos, float scale, float rotY, int variant)
    : position(pos), scaleFactor(scale), rotationY(rotY)
{
    if (variant >= 0)
    {
        variantIndex = variant % 5;
    }
    else
    {
        // Deterministically assign one of the 5 procedural natural variants based on position
        variantIndex = std::abs(static_cast<int>(std::floor(pos.x * 7.31f + pos.z * 13.17f))) % 5;
    }
}

void PineTree::setPosition(const glm::vec3& pos) { position = pos; }
void PineTree::setScale(float scale)             { scaleFactor = scale; }
void PineTree::setRotation(float rotY)           { rotationY = rotY; }

const glm::vec3& PineTree::getPosition() const   { return position; }
float PineTree::getScale() const                 { return scaleFactor; }
float PineTree::getRotation() const              { return rotationY; }

void PineTree::ensureMeshes()
{
    if (!meshesInitialized)
    {
        sharedVariants.reserve(5);
        for (int i = 0; i < 5; ++i)
        {
            // 5 distinct procedural stylized conifer variants matching the reference image:
            // 0: Compact Dwarf Pine
            // 1: Slender Mountain Pine
            // 2: Grand Hero Pine (centerpiece)
            // 3: Wind-Swept Bent Pine
            // 4: Alpine Stunted Pine
            sharedVariants.push_back(std::make_unique<StylizedPineMesh>(i));
        }
        meshesInitialized = true;
    }
}

void PineTree::draw(const Shader& shader, const Cube&, const Pyramid&) const
{
    draw(shader);
}

void PineTree::draw(const Shader& shader) const
{
    ensureMeshes();

    glm::mat4 rootMat(1.0f);
    rootMat = glm::translate(rootMat, position);
    rootMat = glm::rotate(rootMat, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    rootMat = glm::scale(rootMat, glm::vec3(scaleFactor));

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 16.0f);
    shader.setMat4("model", rootMat);
    shader.setVec3("objectColor", glm::vec3(0.0f)); // Enables rich per-vertex bark and foliage colors

    if (variantIndex >= 0 && variantIndex < static_cast<int>(sharedVariants.size()))
    {
        sharedVariants[variantIndex]->draw(shader);
    }
}
