#ifndef MOTION_SYSTEM_H
#define MOTION_SYSTEM_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

// Forward declaration
class Cloud;
class Rain;

// ---------------------------------------------------------------------------
// Time of Day State Machine & Presets
// ---------------------------------------------------------------------------
enum TimeOfDay {
    TIME_NIGHT = 0,
    TIME_SUNSET = 1,
    TIME_DAY = 2
};

// ---------------------------------------------------------------------------
// Environment Preset & State (Celestial positions, atmospheric & light tones)
// ---------------------------------------------------------------------------
struct EnvironmentPreset {
    glm::vec3 skyHorizon;
    glm::vec3 skyZenith;
    glm::vec3 clearColor;
    glm::vec3 sunPos;
    glm::vec3 moonPos;
    glm::vec3 celestialPos;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    glm::vec3 cloudColor;
    glm::vec3 cloudBottomColor;
    glm::vec3 waterBaseColor;
    float starAlpha;
    float moonFactor;   // 1.0 = Moon dominant, 0.0 = Sun dominant
    float sunAlpha;     // 1.0 = Visible Sun, 0.0 = Hidden Sun
    float moonAlpha;    // 1.0 = Visible Moon, 0.0 = Hidden Moon
    float sunsetFactor; // 1.0 = Golden Sunset / Dusk, 0.0 = Day or Night
};

// ---------------------------------------------------------------------------
// Campfire Sparks & Flame Tendril Structures
// ---------------------------------------------------------------------------
struct CampfireSpark {
    glm::vec3 localPos;  // Position relative to campfire root
    glm::vec3 color;     // Emissive spark color (bright gold, fiery amber, incandescent)
    float scale;         // Spark diameter
    float alpha;         // Spark opacity
};

struct FlameTongueTransform {
    glm::mat4 transform;
    glm::vec3 color;
    glm::vec3 emitColor;
};

// ---------------------------------------------------------------------------
// Flame & Firelight Motion State
// Encapsulates the dynamic harmonic transforms of multiple licking flame tongues,
// rising buoyant embers/sparks, and the turbulent combustion flicker of light.
// ---------------------------------------------------------------------------
struct FlameMotionState {
    // Legacy support
    glm::mat4 innerFlameTransform;
    glm::mat4 midFlameTransform;
    glm::mat4 outerFlameTransform;

    // Natural multi-tongue flame anatomy (matching reference illustration)
    glm::mat4 coreTransform;                     // Incandescent white-hot heart
    glm::mat4 mainSpireTransform;                 // Tall central dancing flame spire
    std::vector<FlameTongueTransform> tongues;    // Dynamic licking side & crest flame tendrils

    // Rising buoyant embers/sparks rising into the night air
    std::vector<CampfireSpark> sparks;

    // Dynamic point light
    glm::vec3 lightColor;
    float lightIntensity;
};

// ---------------------------------------------------------------------------
// Cave Magical Motes Motion State
// Encapsulates the 3D undulating trajectories and breathing luminescence
// of floating fairy/bioluminescent light motes inside the mystical cave.
// ---------------------------------------------------------------------------
struct CaveMote {
    glm::vec3 localPos;
    glm::vec3 color;
    float scale;
    float intensity;
};

struct CaveMotesState {
    std::vector<CaveMote> motes;
};

// ---------------------------------------------------------------------------
// Windmill Motion State
// Encapsulates the dynamic angular rotation of the windmill sails/blades
// driven by procedural mountain wind currents.
// ---------------------------------------------------------------------------
struct WindmillMotionState {
    float rotationAngle; // Current rotation angle of the blades in degrees
    float rotationSpeed; // Current angular velocity (deg/sec)
};

// ---------------------------------------------------------------------------
// MotionSystem
// Centralized, dedicated module separating all procedural motion, dynamic
// physics/harmonics, lighting fluctuations, and time synchronization
// from the static mesh rendering code.
// ---------------------------------------------------------------------------
class MotionSystem {
public:
    // =======================================================================
    // 1. CPU-Side Mesh Translation Motion: Clouds
    // =======================================================================
    // Updates horizontal wind drifting across the sky and toroidal boundary wrapping
    static void updateClouds(std::vector<Cloud>& clouds, float dt);

    // =======================================================================
    // 2. CPU-Side Harmonic Matrix Motion: Campfire Flames & Light Flicker
    // =======================================================================
    // Calculates compound multi-frequency sine/cosine harmonics for flame sway,
    // lateral wobble, rotation twist, and vertical stretching.
    // Also computes turbulent point-light color and intensity flickering.
    // speedMultiplier controls the fire animation speed (e.g. 0.5f = slow, 2.0f = fast).
    static FlameMotionState computeCampfireMotion(float time, const glm::mat4& rootMat, float speedMultiplier = 1.0f);

    // =======================================================================
    // 3. Dynamic Lighting Motion: Mystical Cave Bioluminescence
    // =======================================================================
    // Computes breathing sine wave intensity oscillation for subterranean cyan glow
    static float computeCaveLightPulse(float time);

    // =======================================================================
    // 4. Trajectory & Environmental Interpolation Motion: Celestial Bodies
    // =======================================================================
    // Performs smooth exponential ease-out interpolation (lerp) for Sun/Moon
    // sky trajectories and atmospheric color transitions between presets
    static void updateEnvironmentTransition(EnvironmentPreset& current,
                                            const EnvironmentPreset& target,
                                            float dt,
                                            float speed = 3.2f);

    // Computes camera-facing spherical billboard transformation matrix for celestial halos & coronas
    static glm::mat4 computeCelestialBillboard(const glm::vec3& centerPos, const glm::vec3& camPos);

    // =======================================================================
    // 5. GPU Shader Motion Dispatcher: Lake Waves & Star Twinkle
    // =======================================================================
    // Feeds simulation time into shaders to drive:
    //   - water_vertex.glsl: 3D Gerstner wave displacement
    //   - water_fragment.glsl: High-frequency capillary ripples & shoreline foam
    //   - stars_vertex.glsl: Individual star pulsing & twinkling
    static void setShaderTime(Shader& shader, float time);

    // =======================================================================
    // 6. CPU-Side 3D Floating Motion: Mystical Cave Magical Light Points
    // =======================================================================
    // Computes compound 3D harmonic orbits (Lissajous curves) and breathing
    // emissive intensities for magical light motes floating in the cave air.
    static CaveMotesState computeCaveMotesMotion(float time);

    // =======================================================================
    // 7. CPU-Side Rotational Motion: Lakeside Windmill Sails
    // =======================================================================
    // Calculates continuous aerodynamic rotation with subtle atmospheric
    // wind speed turbulence / gusts for the 4-blade windmill rotor.
    static WindmillMotionState computeWindmillMotion(float time);

    // =======================================================================
    // 8. CPU-Side Translation & Recycling Motion: Continuous Rain
    // =======================================================================
    // Updates continuous downward translation of raindrops along a natural
    // diagonal wind vector, performs ground/obstacle collision detection,
    // and resets/recycles raindrops back to the clouds at high altitudes.
    static void updateRain(Rain& rain, float dt, const glm::vec3& windVelocity);
};

#endif // MOTION_SYSTEM_H
