#include "objects/FirewoodProp.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

FirewoodManager::FirewoodManager()
    : m_collectedCount(0)
{
    // 12 natural fallen wood locations scattered around cabin, porch, trees, meadow, and campsite
    m_logs = {
        // Porch front & steps (immediately accessible right outside cabin)
        { glm::vec3(-10.2f, -0.73f,  3.2f),  35.0f, WOOD_LOG_BUNDLE, false, 1.00f }, // 1. Porch steps
        { glm::vec3(-11.5f, -0.73f,  4.5f),  75.0f, WOOD_BRANCH,     false, 1.10f }, // 2. Front yard
        { glm::vec3( -8.5f, -0.73f,  2.8f), -40.0f, WOOD_SINGLE_LOG, false, 1.15f }, // 3. Meadow grass near porch

        // Meadow paths between cabin, campsite & lake
        { glm::vec3( -6.0f, -0.72f,  2.5f),  35.0f, WOOD_BRANCH,     false, 1.05f }, // 4. Trail to campfire
        { glm::vec3( -4.5f, -0.75f,  6.5f),  85.0f, WOOD_LOG_BUNDLE, false, 1.00f }, // 5. Campsite shoreline
        { glm::vec3( -7.8f, -0.76f,  0.5f),  80.0f, WOOD_LOG_BUNDLE, false, 1.05f }, // 6. Meadow lake trail
        { glm::vec3( -1.8f, -0.95f,  5.5f), -15.0f, WOOD_SINGLE_LOG, false, 1.10f }, // 7. Lake shore pebbles

        // North & West pine forest groves around cabin
        { glm::vec3( -9.5f, -0.72f, -4.5f), 115.0f, WOOD_BRANCH,     false, 1.20f }, // 8. North pine clearing
        { glm::vec3(-16.8f, -0.72f, -1.2f), 110.0f, WOOD_LOG_BUNDLE, false, 1.20f }, // 9. West tall pine
        { glm::vec3(-13.8f, -0.70f, -5.8f), -25.0f, WOOD_SINGLE_LOG, false, 1.25f }, // 10. Boulder slope behind cabin
        { glm::vec3(-15.5f, -0.72f,  3.0f),  45.0f, WOOD_BRANCH,     false, 1.10f }, // 11. South-west clearing
        { glm::vec3(-11.2f, -0.73f, -8.5f), 140.0f, WOOD_BRANCH,     false, 1.15f }  // 12. Deep pine forest edge
    };
}

void FirewoodManager::reset()
{
    m_collectedCount = 0;
    for (auto& log : m_logs)
    {
        log.isCollected = false;
    }
}

bool FirewoodManager::tryCollectNear(const glm::vec3& playerPos, float interactRadius, int* outIndex)
{
    float nearestDist = 1e9f;
    int nearestIdx = -1;

    for (size_t i = 0; i < m_logs.size(); ++i)
    {
        if (!m_logs[i].isCollected)
        {
            float dx = playerPos.x - m_logs[i].position.x;
            float dz = playerPos.z - m_logs[i].position.z;
            float horizDist = std::sqrt(dx * dx + dz * dz);
            float vertDist = std::abs(playerPos.y - m_logs[i].position.y);

            // Generous cylindrical interaction volume: horizontal radius and vertical clearance
            if (horizDist <= interactRadius && vertDist <= 4.0f)
            {
                if (horizDist < nearestDist)
                {
                    nearestDist = horizDist;
                    nearestIdx = static_cast<int>(i);
                }
            }
        }
    }

    if (nearestIdx != -1)
    {
        m_logs[nearestIdx].isCollected = true;
        m_collectedCount++;
        if (outIndex) *outIndex = nearestIdx;
        return true;
    }

    return false;
}

bool FirewoodManager::isNearUncollected(const glm::vec3& playerPos, float interactRadius, float* outDist) const
{
    float nearest = 1e9f;
    bool found = false;

    for (const auto& log : m_logs)
    {
        if (!log.isCollected)
        {
            float dx = playerPos.x - log.position.x;
            float dz = playerPos.z - log.position.z;
            float horizDist = std::sqrt(dx * dx + dz * dz);
            float vertDist = std::abs(playerPos.y - log.position.y);

            if (horizDist < nearest)
            {
                nearest = horizDist;
            }
            if (horizDist <= interactRadius && vertDist <= 4.0f)
            {
                found = true;
            }
        }
    }

    if (outDist) *outDist = nearest;
    return found;
}

bool FirewoodManager::getNearestUncollected(const glm::vec3& playerPos, float* outDist, glm::vec3* outPos) const
{
    float nearest = 1e9f;
    int nearestIdx = -1;

    for (size_t i = 0; i < m_logs.size(); ++i)
    {
        if (!m_logs[i].isCollected)
        {
            float dx = playerPos.x - m_logs[i].position.x;
            float dz = playerPos.z - m_logs[i].position.z;
            float horizDist = std::sqrt(dx * dx + dz * dz);

            if (horizDist < nearest)
            {
                nearest = horizDist;
                nearestIdx = static_cast<int>(i);
            }
        }
    }

    if (nearestIdx != -1)
    {
        if (outDist) *outDist = nearest;
        if (outPos) *outPos = m_logs[nearestIdx].position;
        return true;
    }

    return false;
}

void FirewoodManager::draw(const Shader& shader,
                           const Cylinder& unitCylinder,
                           const Cube& unitCube,
                           const Sphere& unitSphere,
                           const Branch& branchMesh,
                           float totalTime) const
{
    for (const auto& log : m_logs)
    {
        if (log.isCollected) continue;

        glm::mat4 m(1.0f);
        m = glm::translate(m, log.position);
        m = glm::rotate(m, glm::radians(log.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        if (log.type == WOOD_BRANCH)
        {
            drawBranchProp(shader, branchMesh, unitCylinder, m, log.scale);
        }
        else if (log.type == WOOD_SINGLE_LOG)
        {
            drawSingleLog(shader, unitCylinder, unitCube, m, log.scale);
        }
        else
        {
            drawLogBundle(shader, unitCylinder, unitCube, m, log.scale);
        }

        // Floating amber ember beacon to easily spot fallen wood in grass
        drawEmberBeacon(shader, unitSphere, m, totalTime);
    }
}

void FirewoodManager::draw(const Shader& shader,
                           const Cylinder& unitCylinder,
                           const Cube& unitCube,
                           float totalTime) const
{
    Sphere defaultSphere(1.0f, 16, 12);
    Branch defaultBranch(1.8f, 0.08f);
    draw(shader, unitCylinder, unitCube, defaultSphere, defaultBranch, totalTime);
}

void FirewoodManager::drawLogBundle(const Shader& shader,
                                    const Cylinder& unitCylinder,
                                    const Cube& unitCube,
                                    const glm::mat4& parentMat,
                                    float scale) const
{
    const glm::vec3 colBark(0.32f, 0.20f, 0.12f);
    const glm::vec3 colWoodCore(0.72f, 0.58f, 0.36f);
    const glm::vec3 colTieRope(0.55f, 0.48f, 0.32f);

    shader.setBool("isEmissive", false);

    // Bottom left log
    glm::mat4 m1 = glm::translate(parentMat, glm::vec3(-0.16f * scale, 0.10f * scale, 0.0f));
    m1 = glm::rotate(m1, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m1 = glm::scale(m1, glm::vec3(0.18f * scale, 1.10f * scale, 0.18f * scale));
    shader.setMat4("model", m1);
    shader.setVec3("objectColor", colBark);
    unitCylinder.draw();

    // Core cut ends
    glm::mat4 me1 = glm::scale(m1, glm::vec3(0.85f, 1.002f, 0.85f));
    shader.setMat4("model", me1);
    shader.setVec3("objectColor", colWoodCore);
    unitCylinder.draw();

    // Bottom right log
    glm::mat4 m2 = glm::translate(parentMat, glm::vec3(0.16f * scale, 0.10f * scale, 0.0f));
    m2 = glm::rotate(m2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m2 = glm::scale(m2, glm::vec3(0.17f * scale, 1.12f * scale, 0.17f * scale));
    shader.setMat4("model", m2);
    shader.setVec3("objectColor", colBark);
    unitCylinder.draw();

    glm::mat4 me2 = glm::scale(m2, glm::vec3(0.85f, 1.002f, 0.85f));
    shader.setMat4("model", me2);
    shader.setVec3("objectColor", colWoodCore);
    unitCylinder.draw();

    // Top stacked log
    glm::mat4 m3 = glm::translate(parentMat, glm::vec3(0.0f, 0.24f * scale, 0.0f));
    m3 = glm::rotate(m3, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m3 = glm::scale(m3, glm::vec3(0.16f * scale, 1.05f * scale, 0.16f * scale));
    shader.setMat4("model", m3);
    shader.setVec3("objectColor", colBark);
    unitCylinder.draw();

    glm::mat4 me3 = glm::scale(m3, glm::vec3(0.85f, 1.002f, 0.85f));
    shader.setMat4("model", me3);
    shader.setVec3("objectColor", colWoodCore);
    unitCylinder.draw();

    // Twine wrapping band around bundle
    glm::mat4 mr = glm::translate(parentMat, glm::vec3(0.0f, 0.15f * scale, 0.0f));
    mr = glm::scale(mr, glm::vec3(0.42f * scale, 0.06f * scale, 0.42f * scale));
    shader.setMat4("model", mr);
    shader.setVec3("objectColor", colTieRope);
    unitCube.draw();
}

void FirewoodManager::drawSingleLog(const Shader& shader,
                                    const Cylinder& unitCylinder,
                                    const Cube& unitCube,
                                    const glm::mat4& parentMat,
                                    float scale) const
{
    const glm::vec3 colDarkBark(0.28f, 0.17f, 0.10f);
    const glm::vec3 colWoodCore(0.68f, 0.54f, 0.34f);
    const glm::vec3 colMoss(0.24f, 0.36f, 0.14f);

    shader.setBool("isEmissive", false);

    // Thick fallen timber trunk
    glm::mat4 m = glm::translate(parentMat, glm::vec3(0.0f, 0.14f * scale, 0.0f));
    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.28f * scale, 1.45f * scale, 0.28f * scale));
    shader.setMat4("model", m);
    shader.setVec3("objectColor", colDarkBark);
    unitCylinder.draw();

    // Cut log ends
    glm::mat4 me = glm::scale(m, glm::vec3(0.88f, 1.002f, 0.88f));
    shader.setMat4("model", me);
    shader.setVec3("objectColor", colWoodCore);
    unitCylinder.draw();

    // Moss patch on top of old log
    glm::mat4 mm = glm::translate(parentMat, glm::vec3(0.0f, 0.26f * scale, 0.10f * scale));
    mm = glm::scale(mm, glm::vec3(0.18f * scale, 0.04f * scale, 0.65f * scale));
    shader.setMat4("model", mm);
    shader.setVec3("objectColor", colMoss);
    unitCube.draw();
}

void FirewoodManager::drawBranchProp(const Shader& shader,
                                     const Branch& branchMesh,
                                     const Cylinder& unitCylinder,
                                     const glm::mat4& parentMat,
                                     float scale) const
{
    const glm::vec3 colBranch(0.30f, 0.19f, 0.11f);
    const glm::vec3 colTwig(0.35f, 0.22f, 0.13f);

    shader.setBool("isEmissive", false);
    shader.setVec3("objectColor", colBranch);

    // Main organic branch mesh
    glm::mat4 mb = glm::translate(parentMat, glm::vec3(0.0f, 0.06f * scale, 0.0f));
    mb = glm::scale(mb, glm::vec3(scale, scale * 0.9f, scale));
    shader.setMat4("model", mb);
    const_cast<Branch&>(branchMesh).draw();

    // Extra side twig fork
    glm::mat4 mt1 = glm::translate(parentMat, glm::vec3(0.25f * scale, 0.08f * scale, 0.15f * scale));
    mt1 = glm::rotate(mt1, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mt1 = glm::rotate(mt1, glm::radians(75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mt1 = glm::scale(mt1, glm::vec3(0.06f * scale, 0.65f * scale, 0.06f * scale));
    shader.setMat4("model", mt1);
    shader.setVec3("objectColor", colTwig);
    unitCylinder.draw();

    // Second side twig
    glm::mat4 mt2 = glm::translate(parentMat, glm::vec3(-0.30f * scale, 0.07f * scale, -0.12f * scale));
    mt2 = glm::rotate(mt2, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mt2 = glm::rotate(mt2, glm::radians(80.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mt2 = glm::scale(mt2, glm::vec3(0.05f * scale, 0.50f * scale, 0.05f * scale));
    shader.setMat4("model", mt2);
    unitCylinder.draw();
}

void FirewoodManager::drawEmberBeacon(const Shader& shader,
                                      const Sphere& unitSphere,
                                      const glm::mat4& parentMat,
                                      float totalTime) const
{
    // Floating glowing ember beacon so player can easily locate fallen wood in meadow
    float floatY = 0.42f + 0.06f * std::sin(totalTime * 3.5f);
    float pulse = 0.85f + 0.35f * std::sin(totalTime * 5.0f);

    glm::vec3 emberCol(1.0f, 0.65f, 0.15f);
    glm::vec3 emitCol(1.0f, 0.55f, 0.08f);

    glm::mat4 mb = glm::translate(parentMat, glm::vec3(0.0f, floatY, 0.0f));
    mb = glm::scale(mb, glm::vec3(0.075f * pulse));

    shader.setMat4("model", mb);
    shader.setVec3("objectColor", emberCol);
    shader.setBool("isEmissive", true);
    shader.setVec3("emissionColor", emitCol * 1.8f);

    unitSphere.draw();
    shader.setBool("isEmissive", false);
}
