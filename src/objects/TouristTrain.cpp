#include "objects/TouristTrain.h"
#include "objects/TerrainMesh.h"
#include "texture/TextureManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static members
std::vector<TrackSample> TouristTrain::s_trackSamples;
float TouristTrain::s_totalTrackLength = 0.0f;
bool TouristTrain::s_trackInitialized = false;

// ===========================================================================
// TrackMesh Implementation
// ===========================================================================
TrackMesh::TrackMesh(const std::vector<TrackSample>& samples)
    : Mesh()
{
    if (samples.empty()) return;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const glm::vec3 colBallast(0.36f, 0.34f, 0.30f);   // Earthy cobblestone & gravel bed
    const glm::vec3 colWoodTie(0.30f, 0.20f, 0.12f);   // Weathered dark timber railway sleepers
    const glm::vec3 colSteelRail(0.48f, 0.50f, 0.54f); // Metallic steel track rails
    const glm::vec3 normUp(0.0f, 1.0f, 0.0f);

    size_t count = samples.size();

    // 1. Continuous Curved Gravel Promenade Roadbed (Width = 2.6m)
    for (size_t i = 0; i < count; ++i)
    {
        size_t next = (i + 1) % count;
        const auto& s1 = samples[i];
        const auto& s2 = samples[next];

        glm::vec3 p1L = s1.pos + s1.right * 1.30f - glm::vec3(0.0f, 0.03f, 0.0f);
        glm::vec3 p1R = s1.pos - s1.right * 1.30f - glm::vec3(0.0f, 0.03f, 0.0f);
        glm::vec3 p2L = s2.pos + s2.right * 1.30f - glm::vec3(0.0f, 0.03f, 0.0f);
        glm::vec3 p2R = s2.pos - s2.right * 1.30f - glm::vec3(0.0f, 0.03f, 0.0f);

        unsigned int b = (unsigned int)vertices.size();
        vertices.push_back({ p1L, normUp, colBallast });
        vertices.push_back({ p1R, normUp, colBallast });
        vertices.push_back({ p2R, normUp, colBallast });
        vertices.push_back({ p2L, normUp, colBallast });

        indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
        indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 3);
    }

    // 2. Wooden Sleepers / Cross-Ties (Placed every ~4 samples ~= 0.85m)
    for (size_t i = 0; i < count; i += 4)
    {
        const auto& s = samples[i];
        glm::vec3 c = s.pos + glm::vec3(0.0f, 0.02f, 0.0f);
        float halfLen = 1.05f; // Along right
        float halfWid = 0.12f; // Along forward
        float h = 0.045f;

        glm::vec3 v0 = c + s.right * halfLen - s.forward * halfWid + normUp * h;
        glm::vec3 v1 = c - s.right * halfLen - s.forward * halfWid + normUp * h;
        glm::vec3 v2 = c - s.right * halfLen + s.forward * halfWid + normUp * h;
        glm::vec3 v3 = c + s.right * halfLen + s.forward * halfWid + normUp * h;

        unsigned int b = (unsigned int)vertices.size();
        vertices.push_back({ v0, normUp, colWoodTie });
        vertices.push_back({ v1, normUp, colWoodTie });
        vertices.push_back({ v2, normUp, colWoodTie });
        vertices.push_back({ v3, normUp, colWoodTie });

        indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
        indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 3);
    }

    // 3. Twin Continuous Steel Rails (Gauge = 1.30m, offset = +-0.65m)
    const float railOffsets[2] = { 0.65f, -0.65f };
    for (int r = 0; r < 2; ++r)
    {
        float ro = railOffsets[r];
        for (size_t i = 0; i < count; ++i)
        {
            size_t next = (i + 1) % count;
            const auto& s1 = samples[i];
            const auto& s2 = samples[next];

            float rw = 0.04f;
            float rh = 0.06f;

            glm::vec3 p1L = s1.pos + s1.right * (ro + rw) + normUp * rh;
            glm::vec3 p1R = s1.pos + s1.right * (ro - rw) + normUp * rh;
            glm::vec3 p2L = s2.pos + s2.right * (ro + rw) + normUp * rh;
            glm::vec3 p2R = s2.pos + s2.right * (ro - rw) + normUp * rh;

            unsigned int b = (unsigned int)vertices.size();
            vertices.push_back({ p1L, normUp, colSteelRail });
            vertices.push_back({ p1R, normUp, colSteelRail });
            vertices.push_back({ p2R, normUp, colSteelRail });
            vertices.push_back({ p2L, normUp, colSteelRail });

            indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
            indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 3);
        }
    }

    setupMesh(vertices, indices);
}

void TrackMesh::draw(const Shader& shader) const
{
    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 28.0f);
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("objectColor", glm::vec3(0.0f)); // let vertex colors shine through
    Mesh::draw();
}

// ===========================================================================
// TouristTrain Spline & Path Generation
// ===========================================================================
void TouristTrain::initTrackCurve()
{
    if (s_trackInitialized) return;

    // 16 Control Points forming an expansive, elegant closed loop circuit
    // Spans across X = -32.5m to +30.5m, and Z = 26.5m to 56.0m (Total length ~175 meters!)
    const std::vector<glm::vec2> controlPoints = {
        // Station Platform area (Z ~ 31.5m, straight heading eastward)
        {  8.5f, 31.5f },
        { 16.5f, 32.0f },
        // Curving southeast through eastern cosmos meadow
        { 24.5f, 34.5f },
        { 30.5f, 39.5f },
        // Climbing onto the rolling green foothills towards the southern ridge
        { 29.5f, 47.0f },
        { 22.5f, 53.0f },
        // Cresting the high southern ridge vista (panoramic lake view)
        { 12.0f, 56.0f },
        {  1.5f, 55.0f },
        // Weaving through highland pines
        { -9.5f, 52.0f },
        { -19.5f, 47.0f },
        // Descending western rolling slopes
        { -28.5f, 41.0f },
        { -32.0f, 33.5f },
        // Western lakeside curve looking across the water to the campsite
        { -27.0f, 27.5f },
        { -17.0f, 26.5f },
        // Approaching the southern flower meadow and station
        {  -7.0f, 28.0f },
        {   0.5f, 30.2f }
    };

    int N = (int)controlPoints.size();
    const int samplesPerSegment = 50;
    int totalSamples = N * samplesPerSegment;

    s_trackSamples.clear();
    s_trackSamples.reserve(totalSamples);

    bool dummyValid = true;

    for (int i = 0; i < N; ++i)
    {
        glm::vec2 pPrev = controlPoints[(i - 1 + N) % N];
        glm::vec2 p0    = controlPoints[i];
        glm::vec2 p1    = controlPoints[(i + 1) % N];
        glm::vec2 p2    = controlPoints[(i + 2) % N];

        for (int j = 0; j < samplesPerSegment; ++j)
        {
            float u = (float)j / (float)samplesPerSegment;
            float u2 = u * u;
            float u3 = u2 * u;

            // Standard Catmull-Rom formulation
            glm::vec2 pos2D = 0.5f * (
                (2.0f * p0) +
                (-pPrev + p1) * u +
                (2.0f * pPrev - 5.0f * p0 + 4.0f * p1 - p2) * u2 +
                (-pPrev + 3.0f * p0 - 3.0f * p1 + p2) * u3
            );

            glm::vec2 tan2D = 0.5f * (
                (-pPrev + p1) +
                2.0f * (2.0f * pPrev - 5.0f * p0 + 4.0f * p1 - p2) * u +
                3.0f * (-pPrev + 3.0f * p0 - 3.0f * p1 + p2) * u2
            );

            glm::vec3 fwd = glm::normalize(glm::vec3(tan2D.x, 0.0f, tan2D.y));
            glm::vec3 rgt = glm::normalize(glm::vec3(fwd.z, 0.0f, -fwd.x));

            float y = TerrainMesh::computeElevation(pos2D.x, pos2D.y, dummyValid) + 0.06f;
            glm::vec3 p3D(pos2D.x, y, pos2D.y);

            s_trackSamples.push_back({ p3D, fwd, rgt, 0.0f });
        }
    }

    // Accumulate distances
    float totalDist = 0.0f;
    s_trackSamples[0].distance = 0.0f;
    for (size_t i = 1; i < s_trackSamples.size(); ++i)
    {
        float d = glm::distance(s_trackSamples[i].pos, s_trackSamples[i - 1].pos);
        totalDist += d;
        s_trackSamples[i].distance = totalDist;
    }
    // Closing segment
    float closingDist = glm::distance(s_trackSamples.back().pos, s_trackSamples[0].pos);
    totalDist += closingDist;
    s_totalTrackLength = totalDist;

    s_trackInitialized = true;
}

TrackSample TouristTrain::sampleTrackAtDistance(float dist)
{
    initTrackCurve();
    if (s_trackSamples.empty())
    {
        return { glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f };
    }

    float L = s_totalTrackLength;
    float s = std::fmod(dist, L);
    if (s < 0.0f) s += L;

    int low = 0;
    int high = (int)s_trackSamples.size() - 1;
    int idx = 0;

    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (s_trackSamples[mid].distance <= s)
        {
            idx = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    int nextIdx = (idx + 1) % (int)s_trackSamples.size();
    const auto& s1 = s_trackSamples[idx];
    const auto& s2 = s_trackSamples[nextIdx];

    float segLen = s2.distance - s1.distance;
    if (segLen < 0.0f) segLen += L;
    float alpha = (segLen > 0.0001f) ? std::clamp((s - s1.distance) / segLen, 0.0f, 1.0f) : 0.0f;

    TrackSample res;
    res.pos = glm::mix(s1.pos, s2.pos, alpha);
    res.forward = glm::normalize(glm::mix(s1.forward, s2.forward, alpha));
    res.right = glm::normalize(glm::mix(s1.right, s2.right, alpha));
    res.distance = s;
    return res;
}

glm::mat4 TouristTrain::getMatrixFromSample(const TrackSample& s, float scaleFactor)
{
    glm::mat4 m(1.0f);
    m[0] = glm::vec4(s.forward * scaleFactor, 0.0f);
    m[1] = glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f) * scaleFactor, 0.0f);
    m[2] = glm::vec4(s.right * scaleFactor, 0.0f);
    m[3] = glm::vec4(s.pos, 1.0f);
    return m;
}

float TouristTrain::getDistanceToTrack(float x, float z)
{
    initTrackCurve();
    float minDistSq = 1e9f;
    for (size_t i = 0; i < s_trackSamples.size(); i += 2)
    {
        float dx = s_trackSamples[i].pos.x - x;
        float dz = s_trackSamples[i].pos.z - z;
        float dSq = dx * dx + dz * dz;
        if (dSq < minDistSq) minDistSq = dSq;
    }
    return std::sqrt(minDistSq);
}

// ===========================================================================
// TouristTrain Class Implementation
// ===========================================================================
TouristTrain::TouristTrain()
    : m_distance(0.0f),
      m_speed(4.6f),         // Relaxed, picturesque sightseeing cruise speed (~16 km/h)
      m_isMoving(true),      // Train moves smoothly by default
      m_wheelAngle(0.0f),
      m_scaleFactor(0.95f),
      m_headlightOn(true),   // Train headlight spotlight active by default
      m_cameraViewMode(0),   // Default to elevated scenic third-person chase view
      m_trackMesh(nullptr)
{
    initTrackCurve();
    m_trackMesh = new TrackMesh(s_trackSamples);
}

TouristTrain::~TouristTrain()
{
    if (m_trackMesh)
    {
        delete m_trackMesh;
        m_trackMesh = nullptr;
    }
}

void TouristTrain::update(float dt)
{
    if (m_isMoving && std::abs(m_speed) > 0.01f)
    {
        m_distance += m_speed * dt;
        if (m_distance >= s_totalTrackLength)
        {
            m_distance = std::fmod(m_distance, s_totalTrackLength);
        }
        else if (m_distance < 0.0f)
        {
            m_distance += s_totalTrackLength;
        }
        // Continuous wheel rolling animation based on wheel radius 0.42m
        m_wheelAngle += (m_speed * dt / 0.42f) * (180.0f / (float)M_PI);
    }
}

glm::vec3 TouristTrain::getLocomotivePosition() const
{
    return sampleTrackAtDistance(m_distance).pos;
}

glm::vec3 TouristTrain::getLocomotiveForward() const
{
    return sampleTrackAtDistance(m_distance).forward;
}

glm::vec3 TouristTrain::getHeadlightPosition() const
{
    TrackSample s = sampleTrackAtDistance(m_distance);
    // Headlights sit on front bumper at +X = 1.78m, +Y = 0.44m relative to locomotive origin
    return s.pos + s.forward * (1.78f * m_scaleFactor) + glm::vec3(0.0f, 0.44f * m_scaleFactor, 0.0f);
}

glm::vec3 TouristTrain::getHeadlightDirection() const
{
    TrackSample s = sampleTrackAtDistance(m_distance);
    // Subtle downward pitch (-0.05 on Y) to sweep tracks and terrain ahead of locomotive
    return glm::normalize(s.forward - glm::vec3(0.0f, 0.05f, 0.0f));
}

glm::vec3 TouristTrain::getCoachPosition() const
{
    TrackSample sCoach = sampleTrackAtDistance(m_distance - 4.15f);
    return sCoach.pos + glm::vec3(0.0f, 1.25f * m_scaleFactor, 0.0f);
}

// ---------------------------------------------------------------------------
// Scenic Ride-Along & Controllable Camera Views (Zero clipping / No black screen)
// ---------------------------------------------------------------------------
glm::vec3 TouristTrain::getRideCameraPosition() const
{
    TrackSample s = sampleTrackAtDistance(m_distance);
    if (m_cameraViewMode == 1)
    {
        // 1. Driver's Front Pilot / Cowcatcher Observation View:
        // Mounted cleanly on the front bumper ahead of boiler and cab
        return s.pos + s.forward * 1.95f + glm::vec3(0.0f, 1.55f, 0.0f);
    }
    else
    {
        // 0. Panoramic Third-Person Follow / Chase View:
        // Elevated smoothly behind and above the train for an unobstructed view of the locomotive and scenery ahead
        return s.pos - s.forward * 5.2f + glm::vec3(0.0f, 3.2f, 0.0f) + s.right * 1.2f;
    }
}

glm::vec3 TouristTrain::getRideCameraLookTarget() const
{
    TrackSample s = sampleTrackAtDistance(m_distance);
    if (m_cameraViewMode == 1)
    {
        // Focus far ahead down the curved railway tracks
        return s.pos + s.forward * 14.0f + glm::vec3(0.0f, 1.20f, 0.0f);
    }
    else
    {
        // Look ahead over the locomotive towards the picturesque landscape
        return s.pos + s.forward * 5.5f + glm::vec3(0.0f, 1.20f, 0.0f);
    }
}

void TouristTrain::toggleCameraView()
{
    m_cameraViewMode = (m_cameraViewMode + 1) % 2;
}

// ---------------------------------------------------------------------------
// User Interactive Controls (Accelerate, Brake, Reverse, Emergency Stop)
// ---------------------------------------------------------------------------
void TouristTrain::processInput(bool accelerate, bool brake, bool emergencyBrake, float dt)
{
    if (emergencyBrake)
    {
        // Rapid mechanical brake to standstill
        if (m_speed > 0.0f)
        {
            m_speed -= 12.0f * dt;
            if (m_speed < 0.0f) m_speed = 0.0f;
        }
        else if (m_speed < 0.0f)
        {
            m_speed += 12.0f * dt;
            if (m_speed > 0.0f) m_speed = 0.0f;
        }

        if (std::abs(m_speed) < 0.05f)
        {
            m_speed = 0.0f;
            m_isMoving = false;
        }
        return;
    }

    if (accelerate)
    {
        m_isMoving = true;
        // Accelerate up to max cruise speed 11.0 m/s (~40 km/h)
        m_speed += 4.5f * dt;
        if (m_speed > 11.0f) m_speed = 11.0f;
    }
    else if (brake)
    {
        // Decelerate / reverse gear
        m_speed -= 5.0f * dt;
        if (m_speed < -3.8f) m_speed = -3.8f; // Reverse speed limit
        if (std::abs(m_speed) > 0.1f)
        {
            m_isMoving = true;
        }
    }
}

void TouristTrain::drawCoupler(const Shader& shader,
                             const Cube& unitCube,
                             const glm::vec3& p1,
                             const glm::vec3& p2) const
{
    const glm::vec3 ironDark(0.12f, 0.12f, 0.12f);
    glm::vec3 mid = 0.5f * (p1 + p2);
    glm::vec3 diff = p2 - p1;
    float len = glm::length(diff);
    if (len < 0.001f) return;

    glm::vec3 fwd = diff / len;
    glm::vec3 rgt = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), fwd));

    glm::mat4 m(1.0f);
    m[0] = glm::vec4(fwd, 0.0f);
    m[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m[2] = glm::vec4(rgt, 0.0f);
    m[3] = glm::vec4(mid, 1.0f);
    m = glm::scale(m, glm::vec3(len, 0.08f, 0.10f));

    shader.setMat4("model", m);
    shader.setVec3("objectColor", ironDark);
    unitCube.draw();
}

void TouristTrain::draw(const Shader& shader,
                         const Cube& unitCube,
                         const Cylinder& unitCylinder,
                         const Sphere& unitSphere,
                         float currentTime) const
{
    // 0. Enforce untextured rendering on railway line and train (pure realistic materials)
    TextureManager::getInstance().setTextureMode(shader, TEX_NONE);

    // 1. Draw the entire elongated curvy promenade railway mesh
    if (m_trackMesh)
    {
        m_trackMesh->draw(shader);
    }

    // 2. Articulated Locomotive & Carriages moving along the spline
    TrackSample sLoco   = sampleTrackAtDistance(m_distance);
    TrackSample sCoach1 = sampleTrackAtDistance(m_distance - 4.15f);
    TrackSample sCoach2 = sampleTrackAtDistance(m_distance - 8.30f);

    glm::mat4 locoMat   = getMatrixFromSample(sLoco, m_scaleFactor);
    glm::mat4 coach1Mat = getMatrixFromSample(sCoach1, m_scaleFactor);
    glm::mat4 coach2Mat = getMatrixFromSample(sCoach2, m_scaleFactor);

    // Draw Locomotive
    drawLocomotive(shader, unitCube, unitCylinder, unitSphere, locoMat, m_wheelAngle, currentTime);

    // Draw Coach 1
    drawCoach(shader, unitCube, unitCylinder, unitSphere, coach1Mat, m_wheelAngle, false);

    // Draw Coach 2 (trailing coach with rear safety marker lanterns)
    drawCoach(shader, unitCube, unitCylinder, unitSphere, coach2Mat, m_wheelAngle, true);

    // Coupler 1 (Rear of Loco to Front of Coach 1)
    glm::vec3 rearLoco = sLoco.pos - sLoco.forward * (1.55f * m_scaleFactor) + glm::vec3(0.0f, 0.26f, 0.0f);
    glm::vec3 frontC1  = sCoach1.pos + sCoach1.forward * (1.55f * m_scaleFactor) + glm::vec3(0.0f, 0.26f, 0.0f);
    drawCoupler(shader, unitCube, rearLoco, frontC1);

    // Coupler 2 (Rear of Coach 1 to Front of Coach 2)
    glm::vec3 rearC1   = sCoach1.pos - sCoach1.forward * (1.55f * m_scaleFactor) + glm::vec3(0.0f, 0.26f, 0.0f);
    glm::vec3 frontC2  = sCoach2.pos + sCoach2.forward * (1.55f * m_scaleFactor) + glm::vec3(0.0f, 0.26f, 0.0f);
    drawCoupler(shader, unitCube, rearC1, frontC2);
}

void TouristTrain::drawLocomotive(const Shader& shader,
                                 const Cube& unitCube,
                                 const Cylinder& unitCylinder,
                                 const Sphere& unitSphere,
                                 const glm::mat4& locoRoot,
                                 float wheelAngle,
                                 float currentTime) const
{
    const glm::vec3 redBody(0.82f, 0.16f, 0.12f);
    const glm::vec3 creamTrim(0.95f, 0.85f, 0.58f);
    const glm::vec3 darkChassis(0.12f, 0.12f, 0.12f);
    const glm::vec3 goldBrass(0.95f, 0.80f, 0.35f);
    const glm::vec3 glassTint(0.12f, 0.16f, 0.20f);
    const glm::vec3 headlightGlow(1.0f, 0.94f, 0.65f);
    const glm::vec3 beaconAmber(1.0f, 0.62f, 0.10f);

    shader.setBool("isEmissive", false);
    shader.setFloat("shininess", 16.0f);

    // --- A. Chassis & Lower Frame ---
    glm::mat4 chassisMat = glm::translate(locoRoot, glm::vec3(0.0f, 0.26f, 0.0f));
    chassisMat = glm::scale(chassisMat, glm::vec3(3.2f, 0.18f, 1.6f));
    shader.setMat4("model", chassisMat);
    shader.setVec3("objectColor", darkChassis);
    unitCube.draw();

    // --- B. Front Bumper / Cowcatcher ---
    glm::mat4 bumperMat = glm::translate(locoRoot, glm::vec3(1.68f, 0.22f, 0.0f));
    bumperMat = glm::scale(bumperMat, glm::vec3(0.20f, 0.22f, 1.70f));
    shader.setMat4("model", bumperMat);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    glm::mat4 bumperStripe = glm::translate(locoRoot, glm::vec3(1.72f, 0.22f, 0.0f));
    bumperStripe = glm::scale(bumperStripe, glm::vec3(0.14f, 0.08f, 1.72f));
    shader.setMat4("model", bumperStripe);
    shader.setVec3("objectColor", darkChassis);
    unitCube.draw();

    // --- C. Front High-Intensity Headlights ---
    const float hlZ[2] = { 0.62f, -0.62f };
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 hlCan = glm::translate(locoRoot, glm::vec3(1.65f, 0.44f, hlZ[i]));
        hlCan = glm::scale(hlCan, glm::vec3(0.22f, 0.20f, 0.20f));
        shader.setMat4("model", hlCan);
        shader.setVec3("objectColor", darkChassis);
        unitCube.draw();

        if (m_headlightOn)
        {
            shader.setBool("isEmissive", true);
            shader.setVec3("emissionColor", headlightGlow * 3.8f);
            shader.setVec3("objectColor", headlightGlow);
        }
        else
        {
            shader.setBool("isEmissive", false);
            shader.setVec3("objectColor", glm::vec3(0.22f, 0.22f, 0.22f));
        }

        glm::mat4 hlLens = glm::translate(locoRoot, glm::vec3(1.78f, 0.44f, hlZ[i]));
        hlLens = glm::rotate(hlLens, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        hlLens = glm::scale(hlLens, glm::vec3(0.15f, 0.04f, 0.15f));
        shader.setMat4("model", hlLens);
        unitCylinder.draw();

        shader.setBool("isEmissive", false);
    }

    // High Hero Central Headlight on front boiler face (High-power beam)
    glm::mat4 heroCan = glm::translate(locoRoot, glm::vec3(1.58f, 1.40f, 0.0f));
    heroCan = glm::rotate(heroCan, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    heroCan = glm::scale(heroCan, glm::vec3(0.36f, 0.22f, 0.36f));
    shader.setMat4("model", heroCan);
    shader.setVec3("objectColor", goldBrass);
    unitCylinder.draw();

    glm::mat4 heroRim = glm::translate(locoRoot, glm::vec3(1.70f, 1.40f, 0.0f));
    heroRim = glm::rotate(heroRim, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    heroRim = glm::scale(heroRim, glm::vec3(0.38f, 0.05f, 0.38f));
    shader.setMat4("model", heroRim);
    shader.setVec3("objectColor", darkChassis);
    unitCylinder.draw();

    if (m_headlightOn)
    {
        shader.setBool("isEmissive", true);
        shader.setVec3("emissionColor", headlightGlow * 4.2f);
        shader.setVec3("objectColor", headlightGlow);
    }
    else
    {
        shader.setBool("isEmissive", false);
        shader.setVec3("objectColor", glm::vec3(0.25f, 0.25f, 0.25f));
    }

    glm::mat4 heroLens = glm::translate(locoRoot, glm::vec3(1.72f, 1.40f, 0.0f));
    heroLens = glm::rotate(heroLens, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    heroLens = glm::scale(heroLens, glm::vec3(0.32f, 0.03f, 0.32f));
    shader.setMat4("model", heroLens);
    unitCylinder.draw();
    shader.setBool("isEmissive", false);

    // --- D. Cylindrical Boiler Nose ---
    shader.setFloat("shininess", 36.0f);
    shader.setVec3("objectColor", redBody);

    glm::mat4 boilerMat = glm::translate(locoRoot, glm::vec3(0.70f, 0.90f, 0.0f));
    boilerMat = glm::rotate(boilerMat, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    boilerMat = glm::scale(boilerMat, glm::vec3(1.10f, 1.60f, 1.10f));
    shader.setMat4("model", boilerMat);
    unitCylinder.draw();

    // Brass accent bands
    shader.setVec3("objectColor", goldBrass);
    const float bandOffsets[2] = { 0.10f, 1.15f };
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 band = glm::translate(locoRoot, glm::vec3(bandOffsets[i], 0.90f, 0.0f));
        band = glm::rotate(band, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        band = glm::scale(band, glm::vec3(1.13f, 0.06f, 1.13f));
        shader.setMat4("model", band);
        unitCylinder.draw();
    }

    // --- E. Front Boiler Nose Face ---
    glm::mat4 bezelMat = glm::translate(locoRoot, glm::vec3(1.52f, 0.90f, 0.0f));
    bezelMat = glm::rotate(bezelMat, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    bezelMat = glm::scale(bezelMat, glm::vec3(1.16f, 0.06f, 1.16f));
    shader.setMat4("model", bezelMat);
    shader.setVec3("objectColor", creamTrim);
    unitCylinder.draw();

    glm::mat4 frontFace = glm::translate(locoRoot, glm::vec3(1.56f, 0.90f, 0.0f));
    frontFace = glm::rotate(frontFace, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    frontFace = glm::scale(frontFace, glm::vec3(0.96f, 0.05f, 0.96f));
    shader.setMat4("model", frontFace);
    shader.setVec3("objectColor", darkChassis);
    unitCylinder.draw();

    glm::mat4 redCenter = glm::translate(locoRoot, glm::vec3(1.60f, 0.90f, 0.0f));
    redCenter = glm::rotate(redCenter, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    redCenter = glm::scale(redCenter, glm::vec3(0.32f, 0.04f, 0.32f));
    shader.setMat4("model", redCenter);
    shader.setVec3("objectColor", redBody);
    unitCylinder.draw();

    // --- F. Vertical Smokestack & Bell ---
    glm::mat4 stackBase = glm::translate(locoRoot, glm::vec3(1.10f, 1.48f, 0.0f));
    stackBase = glm::scale(stackBase, glm::vec3(0.36f, 0.12f, 0.36f));
    shader.setMat4("model", stackBase);
    shader.setVec3("objectColor", goldBrass);
    unitCylinder.draw();

    glm::mat4 stackPipe = glm::translate(locoRoot, glm::vec3(1.10f, 1.72f, 0.0f));
    stackPipe = glm::scale(stackPipe, glm::vec3(0.26f, 0.42f, 0.26f));
    shader.setMat4("model", stackPipe);
    shader.setVec3("objectColor", redBody);
    unitCylinder.draw();

    glm::mat4 stackCap = glm::translate(locoRoot, glm::vec3(1.10f, 2.00f, 0.0f));
    stackCap = glm::scale(stackCap, glm::vec3(0.40f, 0.16f, 0.40f));
    shader.setMat4("model", stackCap);
    shader.setVec3("objectColor", darkChassis);
    unitCylinder.draw();

    glm::mat4 bellMat = glm::translate(locoRoot, glm::vec3(0.45f, 1.54f, 0.0f));
    bellMat = glm::scale(bellMat, glm::vec3(0.24f, 0.22f, 0.24f));
    shader.setMat4("model", bellMat);
    shader.setVec3("objectColor", goldBrass);
    unitSphere.draw();

    // --- G. Driver Cab (Open interior, warm dome light, instrument cluster & crystal glass) ---
    glm::mat4 cabLower = glm::translate(locoRoot, glm::vec3(-0.75f, 0.85f, 0.0f));
    cabLower = glm::scale(cabLower, glm::vec3(1.45f, 1.05f, 1.62f));
    shader.setMat4("model", cabLower);
    shader.setVec3("objectColor", redBody);
    unitCube.draw();

    glm::mat4 cabTrim = glm::translate(locoRoot, glm::vec3(-0.75f, 1.40f, 0.0f));
    cabTrim = glm::scale(cabTrim, glm::vec3(1.48f, 0.08f, 1.65f));
    shader.setMat4("model", cabTrim);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    // 1. Cab Corner Pillars & Window Header Framing
    const float cabPilX[2] = { -0.04f, -1.44f };
    const float cabPilZ[2] = {  0.78f, -0.78f };
    for (int ix = 0; ix < 2; ++ix)
    {
        for (int iz = 0; iz < 2; ++iz)
        {
            glm::mat4 pil = glm::translate(locoRoot, glm::vec3(cabPilX[ix], 1.82f, cabPilZ[iz]));
            pil = glm::scale(pil, glm::vec3(0.10f, 0.80f, 0.10f));
            shader.setMat4("model", pil);
            shader.setVec3("objectColor", creamTrim);
            unitCube.draw();
        }
    }

    glm::mat4 cabHeader = glm::translate(locoRoot, glm::vec3(-0.75f, 2.18f, 0.0f));
    cabHeader = glm::scale(cabHeader, glm::vec3(1.46f, 0.08f, 1.64f));
    shader.setMat4("model", cabHeader);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    // 2. Cab Interior Lights & Features
    // Warm Driver Cab Ceiling Dome Light
    glm::mat4 cabLightMount = glm::translate(locoRoot, glm::vec3(-0.75f, 2.14f, 0.0f));
    cabLightMount = glm::scale(cabLightMount, glm::vec3(0.18f, 0.05f, 0.18f));
    shader.setMat4("model", cabLightMount);
    shader.setVec3("objectColor", goldBrass);
    unitCylinder.draw();

    shader.setBool("isEmissive", true);
    shader.setVec3("emissionColor", glm::vec3(1.0f, 0.90f, 0.65f) * 2.8f);
    shader.setVec3("objectColor", glm::vec3(1.0f, 0.92f, 0.72f));
    glm::mat4 cabLightGlobe = glm::translate(locoRoot, glm::vec3(-0.75f, 2.08f, 0.0f));
    cabLightGlobe = glm::scale(cabLightGlobe, glm::vec3(0.12f, 0.10f, 0.12f));
    shader.setMat4("model", cabLightGlobe);
    unitSphere.draw();
    shader.setBool("isEmissive", false);

    // Driver Control Console & Glowing Instrument Dials
    glm::mat4 dashMat = glm::translate(locoRoot, glm::vec3(-0.08f, 1.48f, 0.0f));
    dashMat = glm::scale(dashMat, glm::vec3(0.12f, 0.22f, 1.20f));
    shader.setMat4("model", dashMat);
    shader.setVec3("objectColor", darkChassis);
    unitCube.draw();

    shader.setBool("isEmissive", true);
    shader.setVec3("emissionColor", glm::vec3(0.35f, 0.92f, 0.75f) * 1.8f);
    shader.setVec3("objectColor", glm::vec3(0.40f, 0.95f, 0.80f));
    const float dialZ[2] = { 0.28f, -0.28f };
    for (int d = 0; d < 2; ++d)
    {
        glm::mat4 dial = glm::translate(locoRoot, glm::vec3(-0.14f, 1.50f, dialZ[d]));
        dial = glm::rotate(dial, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        dial = glm::scale(dial, glm::vec3(0.06f, 0.02f, 0.06f));
        shader.setMat4("model", dial);
        unitCylinder.draw();
    }
    shader.setBool("isEmissive", false);

    // Driver Seat Bench
    glm::mat4 driverSeat = glm::translate(locoRoot, glm::vec3(-1.10f, 1.15f, 0.0f));
    driverSeat = glm::scale(driverSeat, glm::vec3(0.40f, 0.35f, 1.10f));
    shader.setMat4("model", driverSeat);
    shader.setVec3("objectColor", glm::vec3(0.40f, 0.28f, 0.18f));
    unitCube.draw();

    // 3. Driver Cab Crystal Glass Windows (Physical transparent glass with Fresnel sheen)
    const glm::vec3 colGlass(0.72f, 0.88f, 0.98f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.setBool("isGlass", true);
    shader.setFloat("materialAlpha", 0.35f);
    shader.setFloat("shininess", 120.0f);
    shader.setVec3("objectColor", colGlass);

    // Front Windshield Glass Pane
    glm::mat4 windshield = glm::translate(locoRoot, glm::vec3(-0.04f, 1.82f, 0.0f));
    windshield = glm::scale(windshield, glm::vec3(0.03f, 0.64f, 1.44f));
    shader.setMat4("model", windshield);
    unitCube.draw();

    // Side Windows Glass Panes (Left and Right)
    const float sideWinZ[2] = { 0.78f, -0.78f };
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 sideWin = glm::translate(locoRoot, glm::vec3(-0.75f, 1.82f, sideWinZ[i]));
        sideWin = glm::scale(sideWin, glm::vec3(1.30f, 0.64f, 0.03f));
        shader.setMat4("model", sideWin);
        unitCube.draw();
    }

    // Rear Cab Window Glass Pane (Looking into coaches)
    glm::mat4 rearWin = glm::translate(locoRoot, glm::vec3(-1.44f, 1.82f, 0.0f));
    rearWin = glm::scale(rearWin, glm::vec3(0.03f, 0.64f, 1.44f));
    shader.setMat4("model", rearWin);
    unitCube.draw();

    // Disable glass blending for non-transparent elements
    shader.setBool("isGlass", false);
    shader.setFloat("materialAlpha", 1.0f);
    glDisable(GL_BLEND);

    // 4. Exterior Mirrors & Roof
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 mirrorArm = glm::translate(locoRoot, glm::vec3(0.04f, 1.82f, sideWinZ[i] * 1.15f));
        mirrorArm = glm::scale(mirrorArm, glm::vec3(0.04f, 0.04f, 0.22f));
        shader.setMat4("model", mirrorArm);
        shader.setVec3("objectColor", darkChassis);
        unitCube.draw();

        glm::mat4 mirrorHead = glm::translate(locoRoot, glm::vec3(0.04f, 1.85f, sideWinZ[i] * 1.25f));
        mirrorHead = glm::scale(mirrorHead, glm::vec3(0.06f, 0.26f, 0.14f));
        shader.setMat4("model", mirrorHead);
        unitCube.draw();
    }

    glm::mat4 roofMat = glm::translate(locoRoot, glm::vec3(-0.75f, 2.26f, 0.0f));
    roofMat = glm::scale(roofMat, glm::vec3(1.55f, 0.14f, 1.76f));
    shader.setMat4("model", roofMat);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    // Amber roof beacon
    shader.setBool("isEmissive", true);
    shader.setVec3("emissionColor", beaconAmber * 1.8f);
    shader.setVec3("objectColor", beaconAmber);

    glm::mat4 beaconMat = glm::translate(locoRoot, glm::vec3(-0.75f, 2.40f, 0.0f));
    beaconMat = glm::scale(beaconMat, glm::vec3(0.16f, 0.16f, 0.16f));
    shader.setMat4("model", beaconMat);
    unitCylinder.draw();
    shader.setBool("isEmissive", false);

    // --- H. Decorative Arched Mudguards & Rolling Wheels ---
    const float fenderX[2] = { 0.70f, -0.65f };
    const float fenderZ[2] = { 0.84f, -0.84f };
    for (int ix = 0; ix < 2; ++ix)
    {
        for (int iz = 0; iz < 2; ++iz)
        {
            glm::mat4 fender = glm::translate(locoRoot, glm::vec3(fenderX[ix], 0.38f, fenderZ[iz]));
            fender = glm::scale(fender, glm::vec3(0.72f, 0.18f, 0.12f));
            shader.setMat4("model", fender);
            shader.setVec3("objectColor", creamTrim);
            unitCube.draw();

            // Wheel with dynamic rolling spin rotation
            glm::mat4 wheel = glm::translate(locoRoot, glm::vec3(fenderX[ix], 0.22f, fenderZ[iz] * 0.95f));
            wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            wheel = glm::rotate(wheel, glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            wheel = glm::scale(wheel, glm::vec3(0.44f, 0.14f, 0.44f));
            shader.setMat4("model", wheel);
            shader.setVec3("objectColor", darkChassis);
            unitCylinder.draw();

            // Wheel hub
            glm::mat4 hub = glm::translate(locoRoot, glm::vec3(fenderX[ix], 0.22f, fenderZ[iz] * 1.04f));
            hub = glm::rotate(hub, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            hub = glm::scale(hub, glm::vec3(0.18f, 0.06f, 0.18f));
            shader.setMat4("model", hub);
            shader.setVec3("objectColor", goldBrass);
            unitCylinder.draw();
        }
    }
}

void TouristTrain::drawCoach(const Shader& shader,
                            const Cube& unitCube,
                            const Cylinder& unitCylinder,
                            const Sphere& unitSphere,
                            const glm::mat4& coachRoot,
                            float wheelAngle,
                            bool isLastCoach) const
{
    const glm::vec3 redBody(0.82f, 0.16f, 0.12f);
    const glm::vec3 creamTrim(0.95f, 0.85f, 0.58f);
    const glm::vec3 darkChassis(0.12f, 0.12f, 0.12f);
    const glm::vec3 goldBrass(0.95f, 0.80f, 0.35f);
    const glm::vec3 woodSeat(0.42f, 0.28f, 0.18f);
    const glm::vec3 colGlass(0.72f, 0.88f, 0.98f);

    shader.setBool("isEmissive", false);

    // 1. Chassis Frame
    glm::mat4 chassis = glm::translate(coachRoot, glm::vec3(0.0f, 0.24f, 0.0f));
    chassis = glm::scale(chassis, glm::vec3(3.20f, 0.14f, 1.58f));
    shader.setMat4("model", chassis);
    shader.setVec3("objectColor", darkChassis);
    unitCube.draw();

    // 2. Lower Body Wainscoting
    glm::mat4 lowerBody = glm::translate(coachRoot, glm::vec3(0.0f, 0.65f, 0.0f));
    lowerBody = glm::scale(lowerBody, glm::vec3(3.18f, 0.70f, 1.56f));
    shader.setMat4("model", lowerBody);
    shader.setVec3("objectColor", redBody);
    unitCube.draw();

    glm::mat4 waistTrim = glm::translate(coachRoot, glm::vec3(0.0f, 1.02f, 0.0f));
    waistTrim = glm::scale(waistTrim, glm::vec3(3.22f, 0.08f, 1.60f));
    shader.setMat4("model", waistTrim);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    // 3. Interior Timber Floor
    glm::mat4 floorMat = glm::translate(coachRoot, glm::vec3(0.0f, 0.72f, 0.0f));
    floorMat = glm::scale(floorMat, glm::vec3(3.12f, 0.04f, 1.50f));
    shader.setMat4("model", floorMat);
    shader.setVec3("objectColor", glm::vec3(0.28f, 0.20f, 0.14f));
    unitCube.draw();

    // 4. Passenger Seating Benches
    const float seatX[3] = { -0.95f, 0.0f, 0.95f };
    for (int s = 0; s < 3; ++s)
    {
        // Seat cushion
        glm::mat4 seat = glm::translate(coachRoot, glm::vec3(seatX[s], 0.88f, 0.0f));
        seat = glm::scale(seat, glm::vec3(0.42f, 0.22f, 1.30f));
        shader.setMat4("model", seat);
        shader.setVec3("objectColor", woodSeat);
        unitCube.draw();

        // Seat backrest
        glm::mat4 backrest = glm::translate(coachRoot, glm::vec3(seatX[s] - 0.18f, 1.15f, 0.0f));
        backrest = glm::scale(backrest, glm::vec3(0.06f, 0.35f, 1.28f));
        shader.setMat4("model", backrest);
        shader.setVec3("objectColor", woodSeat);
        unitCube.draw();
    }

    // 5. Warm Passenger Ceiling Pendant Lights (Inside Coach behind glass windows)
    for (int s = 0; s < 3; ++s)
    {
        // Ceiling brass fixture
        glm::mat4 fixMat = glm::translate(coachRoot, glm::vec3(seatX[s], 1.96f, 0.0f));
        fixMat = glm::scale(fixMat, glm::vec3(0.18f, 0.04f, 0.18f));
        shader.setMat4("model", fixMat);
        shader.setVec3("objectColor", goldBrass);
        unitCylinder.draw();

        // Glowing warm incandescent glass globe
        shader.setBool("isEmissive", true);
        shader.setVec3("emissionColor", glm::vec3(1.0f, 0.90f, 0.65f) * 3.0f);
        shader.setVec3("objectColor", glm::vec3(1.0f, 0.92f, 0.74f));
        glm::mat4 globeMat = glm::translate(coachRoot, glm::vec3(seatX[s], 1.88f, 0.0f));
        globeMat = glm::scale(globeMat, glm::vec3(0.12f, 0.11f, 0.12f));
        shader.setMat4("model", globeMat);
        unitSphere.draw();
        shader.setBool("isEmissive", false);
    }

    // 6. Window Pillars & Framing
    const float pillarX[4] = { -1.45f, -0.50f, 0.50f, 1.45f };
    for (int p = 0; p < 4; ++p)
    {
        const float pillarZ[2] = { 0.76f, -0.76f };
        for (int z = 0; z < 2; ++z)
        {
            glm::mat4 pil = glm::translate(coachRoot, glm::vec3(pillarX[p], 1.50f, pillarZ[z]));
            pil = glm::scale(pil, glm::vec3(0.12f, 0.90f, 0.12f));
            shader.setMat4("model", pil);
            shader.setVec3("objectColor", creamTrim);
            unitCube.draw();
        }
    }

    // Roof lintel header along sides
    const float lintelZ[2] = { 0.76f, -0.76f };
    for (int z = 0; z < 2; ++z)
    {
        glm::mat4 headerMat = glm::translate(coachRoot, glm::vec3(0.0f, 1.94f, lintelZ[z]));
        headerMat = glm::scale(headerMat, glm::vec3(3.18f, 0.06f, 0.12f));
        shader.setMat4("model", headerMat);
        shader.setVec3("objectColor", creamTrim);
        unitCube.draw();
    }

    // 7. Panoramic Real Glass Windows (Crystal transparent glass with Fresnel reflections)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.setBool("isGlass", true);
    shader.setFloat("materialAlpha", 0.32f);
    shader.setFloat("shininess", 120.0f);
    shader.setVec3("objectColor", colGlass);

    // Side panoramic glass panes
    const float glassZ[2] = { 0.76f, -0.76f };
    for (int z = 0; z < 2; ++z)
    {
        glm::mat4 glass = glm::translate(coachRoot, glm::vec3(0.0f, 1.50f, glassZ[z]));
        glass = glm::scale(glass, glm::vec3(2.82f, 0.78f, 0.02f));
        shader.setMat4("model", glass);
        unitCube.draw();
    }

    // Front & rear vestibule end glass panels
    const float endX[2] = { 1.52f, -1.52f };
    for (int x = 0; x < 2; ++x)
    {
        glm::mat4 endGlass = glm::translate(coachRoot, glm::vec3(endX[x], 1.50f, 0.0f));
        endGlass = glm::scale(endGlass, glm::vec3(0.02f, 0.78f, 1.30f));
        shader.setMat4("model", endGlass);
        unitCube.draw();
    }

    shader.setBool("isGlass", false);
    shader.setFloat("materialAlpha", 1.0f);
    glDisable(GL_BLEND);

    // 8. Rear Ruby Red Safety Marker Lanterns (Trailing coach)
    if (isLastCoach)
    {
        const float markerZ[2] = { 0.70f, -0.70f };
        for (int m = 0; m < 2; ++m)
        {
            glm::mat4 lanternMount = glm::translate(coachRoot, glm::vec3(-1.62f, 1.55f, markerZ[m]));
            lanternMount = glm::scale(lanternMount, glm::vec3(0.12f, 0.16f, 0.12f));
            shader.setMat4("model", lanternMount);
            shader.setVec3("objectColor", darkChassis);
            unitCube.draw();

            shader.setBool("isEmissive", true);
            shader.setVec3("emissionColor", glm::vec3(1.0f, 0.12f, 0.08f) * 3.2f);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.18f, 0.12f));
            glm::mat4 lensMat = glm::translate(coachRoot, glm::vec3(-1.68f, 1.55f, markerZ[m]));
            lensMat = glm::scale(lensMat, glm::vec3(0.09f, 0.09f, 0.09f));
            shader.setMat4("model", lensMat);
            unitSphere.draw();
            shader.setBool("isEmissive", false);
        }
    }

    // 9. Canopy Roof
    glm::mat4 roof = glm::translate(coachRoot, glm::vec3(0.0f, 2.02f, 0.0f));
    roof = glm::scale(roof, glm::vec3(3.35f, 0.16f, 1.74f));
    shader.setMat4("model", roof);
    shader.setVec3("objectColor", creamTrim);
    unitCube.draw();

    // 10. Wheels with Arched Mudguards & Rolling Animation
    const float wheelX[2] = { -0.90f, 0.90f };
    const float wheelZ[2] = { 0.82f, -0.82f };
    for (int ix = 0; ix < 2; ++ix)
    {
        for (int iz = 0; iz < 2; ++iz)
        {
            glm::mat4 fender = glm::translate(coachRoot, glm::vec3(wheelX[ix], 0.36f, wheelZ[iz]));
            fender = glm::scale(fender, glm::vec3(0.65f, 0.16f, 0.10f));
            shader.setMat4("model", fender);
            shader.setVec3("objectColor", creamTrim);
            unitCube.draw();

            glm::mat4 wheel = glm::translate(coachRoot, glm::vec3(wheelX[ix], 0.22f, wheelZ[iz] * 0.95f));
            wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            wheel = glm::rotate(wheel, glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            wheel = glm::scale(wheel, glm::vec3(0.42f, 0.12f, 0.42f));
            shader.setMat4("model", wheel);
            shader.setVec3("objectColor", darkChassis);
            unitCylinder.draw();
        }
    }
}
