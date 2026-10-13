#ifndef AUTUMN_AREA_H
#define AUTUMN_AREA_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"
#include "primitives/Pyramid.h"
#include "primitives/Rock.h"
#include "objects/AutumnTree.h"
#include "objects/TouristTrain.h"
#include "objects/TrainStation.h"
#include "objects/Wildflowers.h"
#include "objects/Treeline.h"

// ---------------------------------------------------------------------------
// LeafCluster Structure
// Represents a grouping of fallen autumn leaves on the ground.
// Grouped into small clusters rather than hundreds of individual objects.
// ---------------------------------------------------------------------------
struct LeafCluster {
    glm::vec3 pos;
    float rotY;
    glm::vec3 scale;
    glm::vec3 color;
};

// ---------------------------------------------------------------------------
// StumpInstance Structure
// Short cylinder environmental detail
// ---------------------------------------------------------------------------
struct StumpInstance {
    glm::vec3 pos;
    float rotY;
    float radius;
    float height;
};

// ---------------------------------------------------------------------------
// LogInstance Structure
// Horizontal fallen log cylinder rotated ~90 degrees
// ---------------------------------------------------------------------------
struct LogInstance {
    glm::vec3 pos;
    float rotY;
    float rotZ;
    glm::vec3 scale; // scale.y is length
};

// ---------------------------------------------------------------------------
// PathSegment Structure
// Curving stone/earth walking path slabs
// ---------------------------------------------------------------------------
struct PathSegment {
    glm::vec3 pos;
    glm::vec3 scale;
    float rotY;
};

// ---------------------------------------------------------------------------
// AutumnLantern Structure
// Cozy warm lantern with post, frame, and glowing inner sphere
// ---------------------------------------------------------------------------
struct AutumnLantern {
    glm::vec3 pos;
    float rotY;
    float height;
};

// ---------------------------------------------------------------------------
// AutumnArea Composite Class
// Manages the entire southern shore autumn landscape:
//   AutumnArea
//   ├── Trees (AutumnTree instances)
//   ├── Path (segments winding from dock to forest)
//   ├── Leaves (grouped flat clusters)
//   ├── Stumps (short cylindrical tree stumps)
//   ├── Logs (horizontal fallen tree trunks)
//   ├── Bench (wooden seat with legs, seat, backrest)
//   ├── Dock (wooden lakeside landing with posts & deck)
//   ├── Lanterns (warm light sources with local coordinates)
//   └── Rocks (small river stones along path and shore)
// ---------------------------------------------------------------------------
class AutumnArea
{
public:
    AutumnArea();

    // Primary draw function reusing existing geometric primitives
    void draw(const Shader& shader,
              const Cube& unitCube,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              const Pyramid& unitPyramid,
              const Rock& rockMesh,
              float currentTime) const;

    void update(float dt) { train.update(dt); }

    // Train interaction & camera modes
    void toggleTrainMoving() { train.toggleMoving(); }
    bool isTrainMoving() const { return train.isMoving(); }
    glm::vec3 getTrainRideCameraPosition() const { return train.getRideCameraPosition(); }
    glm::vec3 getTrainRideCameraLookTarget() const { return train.getRideCameraLookTarget(); }
    void toggleTrainCameraView() { train.toggleCameraView(); }
    int getTrainCameraViewMode() const { return train.getCameraViewMode(); }
    void processTrainInput(bool accelerate, bool brake, bool emergencyBrake, float dt)
    {
        train.processInput(accelerate, brake, emergencyBrake, dt);
    }
    float getTrainSpeed() const { return train.getSpeed(); }
    const TouristTrain& getTrain() const { return train; }
    // Moving Train Headlight Spot Light query & control
    glm::vec3 getTrainHeadlightPos() const { return train.getHeadlightPosition(); }
    glm::vec3 getTrainHeadlightDir() const { return train.getHeadlightDirection(); }
    bool isTrainHeadlightOn() const        { return train.isHeadlightOn(); }
    void toggleTrainHeadlight()            { train.toggleHeadlight(); }
    glm::vec3 getTrainCoachPosition() const { return train.getCoachPosition(); }

    // Get positions and colors of warm lanterns for local lighting
    const std::vector<AutumnLantern>& getLanterns() const { return lanterns; }
    glm::vec3 getPrimaryLanternLightPos() const;
    glm::vec3 getPrimaryLanternLightColor() const;

private:
    std::vector<AutumnTree> trees;
    std::vector<PathSegment> pathSegments;
    std::vector<LeafCluster> leafClusters;
    std::vector<StumpInstance> stumps;
    std::vector<LogInstance> logs;
    std::vector<AutumnLantern> lanterns;

    TouristTrain train;
    TrainStation station;
    Wildflowers wildflowers;
    Treeline treeline;

    struct SmallRock {
        glm::vec3 pos;
        glm::vec3 scale;
        float rotY;
    };
    std::vector<SmallRock> rocks;

    // Bench parameters
    glm::vec3 benchPos;
    float benchRotY;
    float benchScale;

    // Dock parameters
    glm::vec3 dockPos;
    float dockRotY;

    void initTrees();
    void initPath();
    void initLeaves();
    void initStumpsAndLogs();
    void initBench();
    void initDock();
    void initLanterns();
    void initRocks();

    // Sub-renderers
    void drawBench(const Shader& shader, const Cube& unitCube) const;
    void drawDock(const Shader& shader, const Cube& unitCube) const;
    void drawLantern(const Shader& shader, const Cube& unitCube, const Cylinder& unitCylinder, const Sphere& unitSphere, const AutumnLantern& lantern, float currentTime) const;
};

#endif // AUTUMN_AREA_H
