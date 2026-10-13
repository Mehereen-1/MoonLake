#ifndef FIREWOOD_PROP_H
#define FIREWOOD_PROP_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "primitives/Cylinder.h"
#include "primitives/Cube.h"
#include "primitives/Sphere.h"
#include "primitives/Branch.h"

enum FirewoodType {
    WOOD_BRANCH = 0,      // Natural fallen tree branch with twigs
    WOOD_LOG_BUNDLE = 1,  // Stacked chopped firewood logs
    WOOD_SINGLE_LOG = 2   // Large fallen timber log
};

struct FirewoodLog {
    glm::vec3 position;
    float rotationY;
    FirewoodType type;
    bool isCollected;
    float scale;
};

class FirewoodManager
{
public:
    FirewoodManager();

    // Draw all uncollected firewood items scattered in the meadow & forest
    void draw(const Shader& shader,
              const Cylinder& unitCylinder,
              const Cube& unitCube,
              const Sphere& unitSphere,
              const Branch& branchMesh,
              float totalTime) const;

    // Backward-compatible overload
    void draw(const Shader& shader,
              const Cylinder& unitCylinder,
              const Cube& unitCube,
              float totalTime) const;

    // Checks if player is within horizontal interactRadius of any uncollected wood.
    // If yes, collects it, increments count, and returns true.
    bool tryCollectNear(const glm::vec3& playerPos, float interactRadius = 5.0f, int* outIndex = nullptr);

    // Checks distance to nearest uncollected wood
    bool isNearUncollected(const glm::vec3& playerPos, float interactRadius = 5.0f, float* outDist = nullptr) const;

    // Queries nearest uncollected wood distance and world position
    bool getNearestUncollected(const glm::vec3& playerPos, float* outDist = nullptr, glm::vec3* outPos = nullptr) const;

    int getCollectedCount() const { return m_collectedCount; }
    int getTotalCount() const { return static_cast<int>(m_logs.size()); }
    void reset();

    const std::vector<FirewoodLog>& getLogs() const { return m_logs; }

private:
    std::vector<FirewoodLog> m_logs;
    int m_collectedCount;

    void drawLogBundle(const Shader& shader,
                       const Cylinder& unitCylinder,
                       const Cube& unitCube,
                       const glm::mat4& parentMat,
                       float scale) const;

    void drawSingleLog(const Shader& shader,
                       const Cylinder& unitCylinder,
                       const Cube& unitCube,
                       const glm::mat4& parentMat,
                       float scale) const;

    void drawBranchProp(const Shader& shader,
                        const Branch& branchMesh,
                        const Cylinder& unitCylinder,
                        const glm::mat4& parentMat,
                        float scale) const;

    void drawEmberBeacon(const Shader& shader,
                         const Sphere& unitSphere,
                         const glm::mat4& parentMat,
                         float totalTime) const;
};

#endif // FIREWOOD_PROP_H
