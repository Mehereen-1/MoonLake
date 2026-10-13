#include "objects/Campfire.h"
#include "MotionSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Campfire::Campfire(const glm::vec3& pos, float scale)
    : position(pos), scaleFactor(scale)
{
}

void Campfire::setPosition(const glm::vec3& pos) { position = pos; }
void Campfire::setScale(float scale)             { scaleFactor = scale; }

const glm::vec3& Campfire::getPosition() const   { return position; }
float Campfire::getScale() const                 { return scaleFactor; }

glm::mat4 Campfire::getRootMatrix() const
{
    glm::mat4 rootMat(1.0f);
    rootMat = glm::translate(rootMat, position);
    rootMat = glm::scale(rootMat, glm::vec3(scaleFactor));
    return rootMat;
}

glm::vec3 Campfire::getLightPosition() const
{
    return position + glm::vec3(0.0f, 0.65f * scaleFactor, 0.0f);
}

glm::vec3 Campfire::getLightColor(float time) const
{
    return MotionSystem::computeCampfireMotion(time, getRootMatrix()).lightColor;
}

float Campfire::getLightIntensity(float time) const
{
    return MotionSystem::computeCampfireMotion(time, getRootMatrix()).lightIntensity;
}

void Campfire::draw(const Shader& shader,
                    const Cube& unitCube,
                    const Pyramid& unitPyramid,
                    const Sphere& unitSphere,
                    const Rock& unitRock,
                    const FlameMotionState& flameMotion) const
{
    glm::mat4 rootMat = getRootMatrix();

    // -----------------------------------------------------------------------
    // 1. Natural River Stone Fire Ring (14 Varied Boulders, Matching Illustration)
    // -----------------------------------------------------------------------
    shader.setBool("isEmissive", false);
    const glm::vec3 stoneBase(0.28f, 0.27f, 0.31f);
    const glm::vec3 stoneWarm(0.33f, 0.30f, 0.28f);
    const int rockCount = 14;
    const float ringRadius = 1.05f;

    for (int i = 0; i < rockCount; ++i)
    {
        float angle = (float)i * (2.0f * (float)M_PI / rockCount);
        // Subtle radial waviness for organic irregularity
        float rOffset = 0.08f * std::sin((float)i * 2.3f);
        float x = std::cos(angle) * (ringRadius + rOffset);
        float z = std::sin(angle) * (ringRadius + rOffset);

        float rScale = 0.32f + 0.12f * std::sin((float)i * 1.9f + 0.5f);
        float rot = (float)(i * 47);

        glm::mat4 rMat = glm::translate(rootMat, glm::vec3(x, 0.11f, z));
        rMat = glm::rotate(rMat, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
        rMat = glm::rotate(rMat, glm::radians((float)(i % 3 * 8)), glm::vec3(1.0f, 0.0f, 0.0f));
        rMat = glm::scale(rMat, glm::vec3(rScale * 1.15f, rScale * 0.85f, rScale));

        shader.setMat4("model", rMat);
        glm::vec3 stoneCol = (i % 2 == 0) ? stoneBase : stoneWarm;
        shader.setVec3("objectColor", stoneCol * (0.85f + 0.25f * std::sin((float)i * 1.4f)));
        unitRock.draw();
    }

    // -----------------------------------------------------------------------
    // 2. Charred Earth Hearth & Ash Bed
    // -----------------------------------------------------------------------
    const glm::vec3 ashCharcoal(0.10f, 0.08f, 0.08f);
    const glm::vec3 ashGray(0.18f, 0.16f, 0.16f);

    // Flat circular burnt hearth beneath the fire
    glm::mat4 ashMat = glm::translate(rootMat, glm::vec3(0.0f, 0.03f, 0.0f));
    ashMat = glm::scale(ashMat, glm::vec3(1.75f, 0.06f, 1.75f));
    shader.setMat4("model", ashMat);
    shader.setVec3("objectColor", ashCharcoal);
    unitCube.draw();

    glm::mat4 innerAshMat = glm::translate(rootMat, glm::vec3(0.0f, 0.06f, 0.0f));
    innerAshMat = glm::scale(innerAshMat, glm::vec3(1.25f, 0.05f, 1.25f));
    shader.setMat4("model", innerAshMat);
    shader.setVec3("objectColor", ashGray);
    unitCube.draw();

    // -----------------------------------------------------------------------
    // 3. Glowing Incandescent Charcoal & Ember Bed
    // -----------------------------------------------------------------------
    shader.setBool("isEmissive", true);
    const glm::vec3 emberHot(1.0f, 0.40f, 0.06f);
    const glm::vec3 emberDeep(0.95f, 0.20f, 0.02f);

    // Central glowing coal mound
    glm::mat4 emberMat = glm::translate(rootMat, glm::vec3(0.0f, 0.10f, 0.0f));
    emberMat = glm::scale(emberMat, glm::vec3(0.70f, 0.16f, 0.70f));
    shader.setMat4("model", emberMat);
    shader.setVec3("objectColor", emberHot);
    shader.setVec3("emissionColor", emberHot * 1.3f);
    unitSphere.draw();

    // 6 glowing coal chunks clustered between the logs
    for (int i = 0; i < 6; ++i)
    {
        float cAngle = (float)i * (2.0f * (float)M_PI / 6.0f) + 0.3f;
        float cx = std::cos(cAngle) * 0.28f;
        float cz = std::sin(cAngle) * 0.28f;

        glm::mat4 chunkMat = glm::translate(rootMat, glm::vec3(cx, 0.08f, cz));
        chunkMat = glm::rotate(chunkMat, glm::radians((float)(i * 55)), glm::vec3(0.2f, 1.0f, 0.4f));
        chunkMat = glm::scale(chunkMat, glm::vec3(0.18f, 0.10f, 0.16f));

        shader.setMat4("model", chunkMat);
        glm::vec3 cCol = (i % 2 == 0) ? emberHot : emberDeep;
        shader.setVec3("objectColor", cCol);
        shader.setVec3("emissionColor", cCol * 1.2f);
        unitCube.draw();
    }

    // -----------------------------------------------------------------------
    // 4. Burning Firewood Logs (Natural Standing Conical Teepee Formation)
    // -----------------------------------------------------------------------
    // As in the reference illustration, logs lean inward against each other
    // around a circle, meeting in a cone apex over the glowing ember bed.
    shader.setBool("isEmissive", false);
    const glm::vec3 barkColor(0.24f, 0.15f, 0.09f);        // Weathered timber bark
    const glm::vec3 charredInner(0.12f, 0.08f, 0.05f);     // Burnt charcoal inner face
    const glm::vec3 glowingLogLip(0.85f, 0.28f, 0.05f);    // Log tips catching fire

    const int teepeeLogCount = 8;
    const float teepeeRadius = 0.62f;
    const float apexY = 0.68f;

    for (int i = 0; i < teepeeLogCount; ++i)
    {
        float logAngle = (float)i * (360.0f / (float)teepeeLogCount) + (float)(i % 3 * 7);
        float rad = glm::radians(logAngle);

        float baseX = std::cos(rad) * teepeeRadius;
        float baseZ = std::sin(rad) * teepeeRadius;

        // Position mid-point along the leaning log segment
        glm::vec3 pBase(baseX, 0.08f, baseZ);
        glm::vec3 pTop(-baseX * 0.15f, apexY, -baseZ * 0.15f);
        glm::vec3 pMid = (pBase + pTop) * 0.5f;

        glm::mat4 logMat = glm::translate(rootMat, pMid);
        // Rotate towards center and tilt inward at ~48 degrees
        logMat = glm::rotate(logMat, glm::radians(-logAngle + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        logMat = glm::rotate(logMat, glm::radians(48.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        logMat = glm::rotate(logMat, glm::radians((float)(i * 22)), glm::vec3(0.0f, 0.0f, 1.0f));
        logMat = glm::scale(logMat, glm::vec3(0.13f, 0.88f, 0.13f));

        shader.setMat4("model", logMat);
        shader.setVec3("objectColor", (i % 2 == 0) ? barkColor : charredInner);
        unitCube.draw();
    }

    // 3 Cross-base kindling branches bracing the teepee foundation
    const float kindlingAngles[3] = { 25.0f, 140.0f, 260.0f };
    for (int i = 0; i < 3; ++i)
    {
        glm::mat4 kMat = glm::translate(rootMat, glm::vec3(0.0f, 0.10f + i * 0.04f, 0.0f));
        kMat = glm::rotate(kMat, glm::radians(kindlingAngles[i]), glm::vec3(0.0f, 1.0f, 0.0f));
        kMat = glm::rotate(kMat, glm::radians(6.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        kMat = glm::scale(kMat, glm::vec3(0.85f, 0.09f, 0.09f));

        shader.setMat4("model", kMat);
        shader.setVec3("objectColor", charredInner);
        unitCube.draw();
    }

    // -----------------------------------------------------------------------
    // 5. Render Animated Natural Dancing Flames (Additive Volumetric Pyramids)
    // -----------------------------------------------------------------------
    shader.setBool("isEmissive", true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE); // Volumetric fire does not occlude depth; blends smoothly

    // A. Incandescent White-Hot Flame Core (Intense heat inside the teepee)
    glm::vec3 coreColor(1.0f, 0.96f, 0.65f);
    shader.setMat4("model", flameMotion.coreTransform);
    shader.setVec3("objectColor", coreColor);
    shader.setVec3("emissionColor", coreColor * 0.85f);
    unitPyramid.draw();

    // B. Central Soaring Flame Spire (Towering through top of logs)
    glm::vec3 spireColor(1.0f, 0.65f, 0.15f);
    shader.setMat4("model", flameMotion.mainSpireTransform);
    shader.setVec3("objectColor", spireColor);
    shader.setVec3("emissionColor", spireColor * 0.75f);
    unitPyramid.draw();

    // C. Dynamic Multi-Layered Licking Flame Tendrils / Tongues (Volumetric Pyramids)
    for (const auto& tongue : flameMotion.tongues)
    {
        shader.setMat4("model", tongue.transform);
        shader.setVec3("objectColor", tongue.color);
        shader.setVec3("emissionColor", tongue.emitColor);
        unitPyramid.draw();
    }

    // -----------------------------------------------------------------------
    // 6. Rising Buoyant Fire Sparks & Embers (Directly Matching Illustration)
    // -----------------------------------------------------------------------
    for (const auto& spark : flameMotion.sparks)
    {
        glm::mat4 sparkMat = glm::translate(rootMat, spark.localPos);
        sparkMat = glm::scale(sparkMat, glm::vec3(spark.scale));

        shader.setMat4("model", sparkMat);
        shader.setVec3("objectColor", spark.color * spark.alpha);
        shader.setVec3("emissionColor", spark.color * spark.alpha * 1.5f);
        unitSphere.draw();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // Reset emission state
    shader.setBool("isEmissive", false);
    shader.setVec3("objectColor", glm::vec3(0.0f));
}

void Campfire::draw(const Shader& shader,
                    const Cube& unitCube,
                    const Pyramid& unitPyramid,
                    const Sphere& unitSphere,
                    const Rock& unitRock,
                    float time) const
{
    FlameMotionState flameMotion = MotionSystem::computeCampfireMotion(time, getRootMatrix());
    draw(shader, unitCube, unitPyramid, unitSphere, unitRock, flameMotion);
}
