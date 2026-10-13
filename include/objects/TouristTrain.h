#ifndef TOURIST_TRAIN_H
#define TOURIST_TRAIN_H

#include <glm/glm.hpp>
#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"

struct TrackSample {
    glm::vec3 pos;
    glm::vec3 forward;
    glm::vec3 right;
    float distance;
};

// ---------------------------------------------------------------------------
// TrackMesh
// Single high-performance GPU batched mesh representing the entire elongated,
// curvy scenic promenade railway (gravel roadbed, wooden sleeper ties, steel rails).
// ---------------------------------------------------------------------------
class TrackMesh : public Mesh
{
public:
    TrackMesh(const std::vector<TrackSample>& samples);
    virtual ~TrackMesh() = default;

    void draw(const Shader& shader) const;
};

// ---------------------------------------------------------------------------
// TouristTrain
// ---------------------------------------------------------------------------
class TouristTrain
{
public:
    TouristTrain();
    ~TouristTrain();

    void update(float dt);

    void setMoving(bool moving) { m_isMoving = moving; }
    bool isMoving() const       { return m_isMoving; }
    void toggleMoving()         { m_isMoving = !m_isMoving; }

    void setSpeed(float speed)  { m_speed = speed; }
    float getSpeed() const      { return m_speed; }

    glm::vec3 getLocomotivePosition() const;
    glm::vec3 getLocomotiveForward() const;
    glm::vec3 getRideCameraPosition() const;
    glm::vec3 getRideCameraLookTarget() const;

    // Moving Spot Light & Coach Light query methods
    glm::vec3 getHeadlightPosition() const;
    glm::vec3 getHeadlightDirection() const;
    glm::vec3 getCoachPosition() const;
    bool isHeadlightOn() const { return m_headlightOn; }
    void toggleHeadlight()     { m_headlightOn = !m_headlightOn; }
    void setHeadlightOn(bool on) { m_headlightOn = on; }

    // User Interactive Controls & Dynamic Camera Views
    void processInput(bool accelerate, bool brake, bool emergencyBrake, float dt);
    void toggleCameraView();
    int getCameraViewMode() const { return m_cameraViewMode; }

    // Static track query helper for props and wildflowers
    static float getDistanceToTrack(float x, float z);

    void draw(const Shader& shader,
              const Cube& unitCube,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              float currentTime) const;

private:
    float m_distance;
    float m_speed;
    bool m_isMoving;
    float m_wheelAngle;
    float m_scaleFactor;
    bool m_headlightOn;
    int m_cameraViewMode; // 0 = Third-Person Follow / Chase View, 1 = Front Driver/Pilot View

    static std::vector<TrackSample> s_trackSamples;
    static float s_totalTrackLength;
    static bool s_trackInitialized;

    static void initTrackCurve();
    static TrackSample sampleTrackAtDistance(float dist);
    static glm::mat4 getMatrixFromSample(const TrackSample& s, float scaleFactor);

    void drawLocomotive(const Shader& shader,
                        const Cube& unitCube,
                        const Cylinder& unitCylinder,
                        const Sphere& unitSphere,
                        const glm::mat4& locoRoot,
                        float wheelAngle,
                        float currentTime) const;

    void drawCoach(const Shader& shader,
                   const Cube& unitCube,
                   const Cylinder& unitCylinder,
                   const Sphere& unitSphere,
                   const glm::mat4& coachRoot,
                   float wheelAngle,
                   bool isLastCoach = false) const;

    void drawCoupler(const Shader& shader,
                     const Cube& unitCube,
                     const glm::vec3& p1,
                     const glm::vec3& p2) const;

    TrackMesh* m_trackMesh;
};

#endif // TOURIST_TRAIN_H
