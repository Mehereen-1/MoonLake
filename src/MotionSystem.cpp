#include "MotionSystem.h"
#include "objects/Cloud.h"
#include "objects/Rain.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// ===========================================================================
// 1. CPU-Side Mesh Translation Motion: Clouds
// ===========================================================================
void MotionSystem::updateClouds(std::vector<Cloud>& clouds, float dt)
{
    for (auto& cloud : clouds)
    {
        cloud.update(dt);
    }
}

// ===========================================================================
// 2. CPU-Side Harmonic Matrix Motion: Campfire Flames & Light Flicker
// ===========================================================================
// ===========================================================================
// 2. CPU-Side Harmonic Matrix Motion: Campfire Flames, Tendrils & Sparks
// ===========================================================================
FlameMotionState MotionSystem::computeCampfireMotion(float time, const glm::mat4& rootMat, float speedMultiplier)
{
    FlameMotionState state;

    // Apply manual fire speed multiplier to the simulation clock
    float tSim = time * speedMultiplier;

    // --- Dynamic Thermal Combustion Harmonics ---
    // Multi-frequency sinusoidal interference creates fluid, non-repeating organic fire motion
    float tRapid = tSim * 24.0f;
    float tMedium = tSim * 14.0f;
    float tSlow = tSim * 7.5f;

    float flutter1 = std::sin(tRapid);
    float flutter2 = std::cos(tRapid * 1.35f);
    float swayX = std::sin(tMedium) * 0.7f + std::sin(tSlow * 1.8f) * 0.3f;
    float swayZ = std::cos(tMedium * 1.15f) * 0.7f + std::cos(tSlow * 1.4f) * 0.3f;
    float breathY = 1.0f + 0.16f * std::sin(tRapid * 0.85f) + 0.10f * std::cos(tMedium * 1.4f);

    // -----------------------------------------------------------------------
    // A. Incandescent White-Hot Flame Core (Intense internal heat)
    // -----------------------------------------------------------------------
    glm::mat4 coreMat = glm::translate(rootMat, glm::vec3(swayX * 0.02f, 0.22f, swayZ * 0.02f));
    coreMat = glm::rotate(coreMat, glm::radians(swayX * 6.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    coreMat = glm::scale(coreMat, glm::vec3(0.48f, 0.75f * breathY, 0.48f));
    state.coreTransform = coreMat;
    state.innerFlameTransform = coreMat; // Legacy compatibility

    // -----------------------------------------------------------------------
    // B. Main Soaring Flame Spire (Reaching upward through the center of logs)
    // -----------------------------------------------------------------------
    float spireHeight = 1.55f * (1.0f + 0.22f * std::sin(tMedium * 1.2f) + 0.12f * std::cos(tRapid * 0.9f));
    glm::mat4 spireMat = glm::translate(rootMat, glm::vec3(swayX * 0.06f, 0.30f, swayZ * 0.05f));
    spireMat = glm::rotate(spireMat, glm::radians(std::sin(tSlow) * 18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    spireMat = glm::rotate(spireMat, glm::radians(swayX * 12.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    spireMat = glm::rotate(spireMat, glm::radians(-swayZ * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    spireMat = glm::scale(spireMat, glm::vec3(0.58f, spireHeight, 0.58f));
    state.mainSpireTransform = spireMat;
    state.midFlameTransform = spireMat; // Legacy compatibility

    // -----------------------------------------------------------------------
    // C. Multi-Layered Volumetric Flame Pyramid Assembly (54 Pyramids)
    // -----------------------------------------------------------------------
    state.tongues.reserve(54);

    // =======================================================================
    // TIER 1: Inner White-Hot Combustion Cores (8 Pyramids)
    // Deep inside the burning logs around the coal bed. High-frequency sine shimmer.
    // =======================================================================
    for (int i = 0; i < 8; ++i)
    {
        float angle = (float)i * (360.0f / 8.0f) + 15.0f;
        float rad = glm::radians(angle);
        float radius = 0.05f + 0.04f * std::sin((float)i * 1.7f);

        float freq = 20.0f + (float)(i * 3);
        float phase = (float)i * 0.9f;
        float flutter = std::sin(tSim * freq + phase);
        float flutterCos = std::cos(tSim * (freq * 1.15f) + phase);

        float px = std::cos(rad) * radius + swayX * 0.02f + 0.015f * flutter;
        float pz = std::sin(rad) * radius + swayZ * 0.02f + 0.015f * flutterCos;
        float py = 0.12f + 0.03f * std::sin(tSim * 14.0f + (float)i);

        float hScale = (0.55f + 0.15f * std::sin((float)i * 2.1f)) * (1.0f + 0.22f * flutter);
        float wScale = (0.32f + 0.06f * std::cos((float)i * 1.8f)) * (1.0f - 0.10f * flutter);

        glm::mat4 mat = glm::translate(rootMat, glm::vec3(px, py, pz));
        mat = glm::rotate(mat, glm::radians(angle + flutter * 8.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(flutter * 6.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        mat = glm::scale(mat, glm::vec3(wScale, hScale, wScale));

        glm::vec3 col(1.0f, 0.94f, 0.60f);
        glm::vec3 emit(0.85f, 0.78f, 0.48f);
        state.tongues.push_back({ mat, col, emit });
    }

    // =======================================================================
    // TIER 2: Central Ascending Flame Spires (10 Pyramids)
    // Towering upward through the center of the teepee log cone.
    // Driven by convective breathing and fluid volume pinching using sine.
    // =======================================================================
    for (int i = 0; i < 10; ++i)
    {
        float angle = (float)i * (360.0f / 10.0f) + 32.0f;
        float rad = glm::radians(angle);
        float radius = 0.08f + 0.06f * std::sin((float)i * 1.4f);

        float freqH = 11.0f + (float)(i % 5 * 2.5f);
        float phaseH = (float)i * 1.25f;

        // Sine wave vertical surge
        float surge = std::sin(tSim * freqH + phaseH);
        float surgeHarmonic = std::sin(tSim * (freqH * 2.1f) + phaseH);

        // Fluid volume conservation: stretches taller while narrowing base
        float hBase = 1.05f + 0.45f * std::sin((float)i * 1.9f + 0.5f);
        float hScale = hBase * (1.0f + 0.28f * surge + 0.12f * surgeHarmonic);
        float wScale = 0.28f * (1.0f - 0.16f * surge);

        float px = std::cos(rad) * radius + swayX * (0.04f + 0.02f * (float)i / 10.0f);
        float pz = std::sin(rad) * radius + swayZ * (0.04f + 0.02f * (float)i / 10.0f);
        float py = 0.20f + 0.04f * surgeHarmonic;

        float tilt = (6.0f + 4.0f * std::sin((float)i * 2.5f)) + 5.0f * surge;
        float twist = (float)(i * 36) + 15.0f * std::sin(tSim * 8.0f + (float)i);

        glm::mat4 mat = glm::translate(rootMat, glm::vec3(px, py, pz));
        mat = glm::rotate(mat, glm::radians(twist), glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(tilt), glm::vec3(std::cos(rad), 0.0f, std::sin(rad)));
        mat = glm::scale(mat, glm::vec3(wScale, hScale, wScale));

        glm::vec3 col = (i % 2 == 0) ? glm::vec3(1.0f, 0.72f, 0.16f) : glm::vec3(1.0f, 0.58f, 0.10f);
        glm::vec3 emit = col * 0.75f;
        state.tongues.push_back({ mat, col, emit });
    }

    // =======================================================================
    // TIER 3: Radial Dancing & Licking Tongues (20 Pyramids)
    // Distributed around the perimeter, weaving between the 8 logs.
    // Sine-based outward leaning, lateral sway, and flame tongue curling.
    // =======================================================================
    for (int i = 0; i < 20; ++i)
    {
        float angle = (float)i * (360.0f / 20.0f) + 9.0f * std::sin((float)i * 3.1f);
        float rad = glm::radians(angle);
        float radius = 0.20f + 0.10f * std::sin((float)i * 2.3f);

        float freq = 16.0f + (float)(i % 7 * 2.2f);
        float phase = (float)i * 0.85f;
        float wave = std::sin(tSim * freq + phase);
        float waveCos = std::cos(tSim * (freq * 1.25f) + phase);

        float px = std::cos(rad) * (radius + 0.03f * wave) + swayX * 0.05f;
        float pz = std::sin(rad) * (radius + 0.03f * waveCos) + swayZ * 0.05f;
        float py = 0.15f + 0.03f * wave;

        float hScale = (0.75f + 0.35f * std::sin((float)i * 1.5f)) * (1.0f + 0.30f * wave);
        float wScale = (0.24f + 0.06f * std::cos((float)i * 2.0f)) * (1.0f - 0.14f * wave);

        // Outward lean away from campfire center
        float outwardLean = 18.0f + 10.0f * std::sin((float)i * 2.8f) + 12.0f * wave;
        float curlRoll = 8.0f * waveCos;

        glm::mat4 mat = glm::translate(rootMat, glm::vec3(px, py, pz));
        mat = glm::rotate(mat, rad, glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(outwardLean), glm::vec3(1.0f, 0.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(curlRoll), glm::vec3(0.0f, 0.0f, 1.0f));
        mat = glm::scale(mat, glm::vec3(wScale, hScale, wScale));

        // Saturated warm campfire orange to deep amber
        float colMix = 0.5f + 0.5f * std::sin((float)i * 1.3f);
        glm::vec3 col = glm::mix(glm::vec3(1.0f, 0.45f, 0.06f), glm::vec3(0.96f, 0.32f, 0.04f), colMix);
        glm::vec3 emit = col * 0.65f;
        state.tongues.push_back({ mat, col, emit });
    }

    // =======================================================================
    // TIER 4: Leaping Flame Tips & Dissipating Micro-Flames (16 Pyramids)
    // Detached flame pockets soaring above the log apex, cycling with sin(pi * tau).
    // =======================================================================
    for (int i = 0; i < 16; ++i)
    {
        float speed = 0.85f + (float)((i * 13) % 7) * 0.12f;
        float offset = (float)(i * 29) / 16.0f;
        float tau = std::fmod(tSim * speed + offset, 1.0f); // Lifecycle [0, 1]

        // Smooth sine wave envelope: emerges, reaches peak intensity, then tapers to 0
        float envelope = std::sin(tau * 3.14159265f);

        float angle = (float)i * (360.0f / 16.0f) + tau * 45.0f;
        float rad = glm::radians(angle);
        float radius = 0.08f + tau * 0.22f; // Expanding vortex

        float px = std::cos(rad) * radius + swayX * (0.06f + 0.08f * tau) + 0.025f * std::sin(tSim * 28.0f + (float)i);
        float pz = std::sin(rad) * radius + swayZ * (0.06f + 0.08f * tau) + 0.025f * std::cos(tSim * 26.0f + (float)i);
        float py = 0.45f + tau * 1.10f; // Climbing vertically

        float wScale = (0.16f + 0.06f * std::sin((float)i * 2.2f)) * envelope;
        float hScale = (0.50f + 0.25f * std::cos((float)i * 1.7f)) * envelope;

        glm::mat4 mat = glm::translate(rootMat, glm::vec3(px, py, pz));
        mat = glm::rotate(mat, rad, glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::rotate(mat, glm::radians(15.0f + 12.0f * std::sin(tSim * 22.0f + (float)i)), glm::vec3(1.0f, 0.0f, 0.0f));
        mat = glm::scale(mat, glm::vec3(wScale, hScale, wScale));

        glm::vec3 col = glm::mix(glm::vec3(1.0f, 0.42f, 0.06f), glm::vec3(0.88f, 0.18f, 0.02f), tau);
        glm::vec3 emit = col * (0.60f * envelope);
        state.tongues.push_back({ mat, col, emit });
    }

    state.outerFlameTransform = state.tongues[3].transform;

    // -----------------------------------------------------------------------
    // D. Rising Buoyant Embers & Sparks (Matching Reference Illustration)
    // -----------------------------------------------------------------------
    const int sparkCount = 22;
    state.sparks.reserve(sparkCount);

    for (int i = 0; i < sparkCount; ++i)
    {
        // Unique deterministic speed and offset per spark, scaled by speedMultiplier
        float sparkSpeed = (0.55f + ((i * 17) % 10) * 0.06f) * speedMultiplier;
        float offset = (float)(i * 37) / (float)sparkCount;
        float cycle = std::fmod(time * sparkSpeed + offset, 1.0f); // Normalized lifetime [0, 1]

        // Vertical ascent with thermal convection acceleration
        float y = 0.38f + cycle * 1.85f;

        // Radial expansion with vortex swirl
        float r = 0.06f + cycle * 0.42f;
        float baseTheta = (float)i * (2.0f * 3.14159f / sparkCount);
        float theta = baseTheta + cycle * 3.8f + 0.35f * std::sin(tSim * 4.0f + i);

        // Turbulent lateral wobble
        float x = r * std::cos(theta) + 0.035f * std::sin(tSim * 9.0f + i * 2.1f);
        float z = r * std::sin(theta) + 0.035f * std::cos(tSim * 8.0f + i * 1.7f);

        glm::vec3 colBase = (cycle < 0.45f)
            ? glm::mix(glm::vec3(1.0f, 0.95f, 0.50f), glm::vec3(1.0f, 0.55f, 0.10f), cycle / 0.45f)
            : glm::mix(glm::vec3(1.0f, 0.55f, 0.10f), glm::vec3(0.95f, 0.22f, 0.04f), (cycle - 0.45f) / 0.55f);

        float twinkle = 0.70f + 0.30f * std::sin(tSim * 30.0f + i * 5.0f);
        float alpha = (1.0f - cycle * cycle) * twinkle;
        float sScale = (0.024f + ((i * 7) % 5) * 0.005f) * (1.0f - cycle * 0.4f);

        state.sparks.push_back({ glm::vec3(x, y, z), colBase * (1.2f + twinkle * 0.6f), sScale, alpha });
    }

    // -----------------------------------------------------------------------
    // E. Dynamic Firelight Flicker Calculations
    // -----------------------------------------------------------------------
    float flicker = 1.0f
        + 0.14f * std::sin(tSim * 18.0f)
        + 0.09f * std::cos(tSim * 34.0f + 1.1f)
        + 0.06f * std::sin(tSim * 52.0f + 2.4f);

    state.lightColor = glm::vec3(1.0f, 0.60f, 0.18f) * flicker;

    float pulse = 1.0f + 0.15f * std::sin(tSim * 22.0f);
    state.lightIntensity = 3.6f * pulse;

    return state;
}

// ===========================================================================
// 3. Dynamic Lighting Motion: Mystical Cave Bioluminescence
// ===========================================================================
float MotionSystem::computeCaveLightPulse(float time)
{
    // Gentle breathing pulse equation at 2.4 rad/s
    return 0.88f + 0.12f * std::sin(time * 2.4f);
}

// ===========================================================================
// 4. Trajectory & Environmental Interpolation Motion: Celestial Bodies
// ===========================================================================
void MotionSystem::updateEnvironmentTransition(EnvironmentPreset& current,
                                               const EnvironmentPreset& target,
                                               float dt,
                                               float speed)
{
    float lerpFactor = glm::clamp(dt * speed, 0.0f, 1.0f);

    current.skyHorizon     = glm::mix(current.skyHorizon,     target.skyHorizon,     lerpFactor);
    current.skyZenith      = glm::mix(current.skyZenith,      target.skyZenith,      lerpFactor);
    current.clearColor     = glm::mix(current.clearColor,     target.clearColor,     lerpFactor);
    current.sunPos         = glm::mix(current.sunPos,         target.sunPos,         lerpFactor);
    current.moonPos        = glm::mix(current.moonPos,        target.moonPos,        lerpFactor);
    current.celestialPos   = glm::mix(current.celestialPos,   target.celestialPos,   lerpFactor);
    current.lightColor     = glm::mix(current.lightColor,     target.lightColor,     lerpFactor);
    current.ambientColor   = glm::mix(current.ambientColor,   target.ambientColor,   lerpFactor);
    current.cloudColor     = glm::mix(current.cloudColor,     target.cloudColor,     lerpFactor);
    current.cloudBottomColor = glm::mix(current.cloudBottomColor, target.cloudBottomColor, lerpFactor);
    current.waterBaseColor = glm::mix(current.waterBaseColor, target.waterBaseColor, lerpFactor);
    current.starAlpha      = glm::mix(current.starAlpha,      target.starAlpha,      lerpFactor);
    current.moonFactor     = glm::mix(current.moonFactor,     target.moonFactor,     lerpFactor);
    current.sunAlpha       = glm::mix(current.sunAlpha,       target.sunAlpha,       lerpFactor);
    current.moonAlpha      = glm::mix(current.moonAlpha,      target.moonAlpha,      lerpFactor);
    current.sunsetFactor   = glm::mix(current.sunsetFactor,   target.sunsetFactor,   lerpFactor);
}

glm::mat4 MotionSystem::computeCelestialBillboard(const glm::vec3& centerPos, const glm::vec3& camPos)
{
    glm::vec3 look = camPos - centerPos;
    if (glm::length(look) > 0.0001f)
        look = glm::normalize(look);
    else
        look = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 upRef(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(upRef, look);
    if (glm::length(right) < 0.001f)
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    else
        right = glm::normalize(right);

    glm::vec3 up = glm::cross(look, right);

    glm::mat4 model(1.0f);
    model[0] = glm::vec4(right, 0.0f);
    model[1] = glm::vec4(up, 0.0f);
    model[2] = glm::vec4(look, 0.0f);
    // Offset slightly behind the 3D sphere along the line of sight
    model[3] = glm::vec4(centerPos - look * 0.12f, 1.0f);
    return model;
}

// ===========================================================================
// 5. GPU Shader Motion Dispatcher: Lake Waves & Star Twinkle
// ===========================================================================
void MotionSystem::setShaderTime(Shader& shader, float time)
{
    shader.setFloat("time", time);
}

// ===========================================================================
// 6. CPU-Side 3D Floating Motion: Mystical Cave Magical Light Points
// "and where did the glowing moving elements inside the cave go?"
// ===========================================================================
CaveMotesState MotionSystem::computeCaveMotesMotion(float time)
{
    CaveMotesState state;
    const int numMotes = 48;
    state.motes.reserve(numMotes);

    const glm::vec3 silverPurple(0.88f, 0.80f, 1.0f);
    const glm::vec3 biolumTeal(0.55f, 0.95f, 0.88f);
    const glm::vec3 warmGold(1.0f, 0.85f, 0.40f);
    const glm::vec3 palette[3] = { silverPurple, biolumTeal, warmGold };

    for (int i = 0; i < numMotes; ++i)
    {
        float tNorm = (float)i / (float)(numMotes - 1);
        float baseZ = -1.2f - tNorm * 23.0f; // Spans from entrance to treasure sanctum

        float s = (std::max)(0.0f, 0.55f - baseZ);
        float blend = (std::min)(1.0f, s / 1.6f);
        blend = blend * blend * (3.0f - 2.0f * blend);
        float xc = (0.42f * std::sin(s * 0.17f) + 0.24f * std::sin(s * 0.38f + 0.5f) - 0.10f * std::cos(s * 0.62f)) * blend;

        // Spread across horizontal width [-0.85m, 0.85m]
        float xOff = 0.85f * std::sin((float)i * 1.85f + 0.4f);
        float baseY = 0.95f + 0.95f * (0.5f + 0.5f * std::cos((float)i * 2.3f));

        // 3D smooth compound Lissajous orbital drift
        float freqX = 1.0f + (float)(i % 5) * 0.25f;
        float freqY = 1.2f + (float)(i % 4) * 0.30f;
        float freqZ = 0.9f + (float)(i % 6) * 0.20f;

        float lx = xc + xOff + 0.32f * std::sin(time * freqX + (float)i * 1.1f);
        float ly = baseY + 0.28f * std::cos(time * freqY + (float)i * 1.7f);
        float lz = baseZ + 0.35f * std::sin(time * freqZ + (float)i * 2.3f);

        // Organic twinkling intensity
        float twinkleSpeed = 2.8f + (float)(i % 7) * 0.4f;
        float pulseA = 0.5f * (std::sin(time * twinkleSpeed + (float)i * 1.5f) + 1.0f);
        float pulseB = 0.5f * (std::cos(time * (twinkleSpeed * 1.6f) + (float)i * 0.8f) + 1.0f);
        float combinedPulse = 0.65f * pulseA + 0.35f * pulseB;

        float scale = (0.016f + (float)(i % 4) * 0.003f) * (0.80f + 0.35f * combinedPulse);
        float intensity = 0.7f + 2.4f * combinedPulse;

        glm::vec3 col = palette[i % 3];
        state.motes.push_back({ glm::vec3(lx, ly, lz), col, scale, intensity });
    }

    return state;
}

// ===========================================================================
// 7. CPU-Side Rotational Motion: Lakeside Windmill Sails
// ===========================================================================
WindmillMotionState MotionSystem::computeWindmillMotion(float time)
{
    // Base steady rotational speed: ~14 RPM = 84.0 deg/sec
    // Modulated by gentle mountain breeze turbulence for organic realism
    const float baseAngularSpeed = 84.0f; // degrees per second
    float angle = baseAngularSpeed * time + 18.0f * std::sin(time * 0.40f) + 6.0f * std::sin(time * 0.95f);
    float instantaneousSpeed = baseAngularSpeed + 18.0f * 0.40f * std::cos(time * 0.40f) + 6.0f * 0.95f * std::cos(time * 0.95f);

    return { angle, instantaneousSpeed };
}

// ===========================================================================
// 8. CPU-Side Translation & Recycling Motion: Continuous Rain
// ===========================================================================
void MotionSystem::updateRain(Rain& rain, float dt, const glm::vec3& windVelocity)
{
    // Fall direction vector: primarily downward with natural diagonal slant
    const glm::vec3 fallVector = glm::vec3(windVelocity.x, -22.5f, windVelocity.z);
    const glm::vec3 normFallDir = glm::normalize(fallVector);
    auto& drops = rain.getDrops();

    const float minX = Rain::BOUNDS_MIN_X;
    const float maxX = Rain::BOUNDS_MAX_X;
    const float minZ = Rain::BOUNDS_MIN_Z;
    const float maxZ = Rain::BOUNDS_MAX_Z;

    // Fast LCG random generator for continuous uniform redistribution across full landscape
    static unsigned int s_rainSeed = 22695477u;

    for (size_t i = 0; i < drops.size(); ++i)
    {
        auto& drop = drops[i];

        // 1. Continuous downward & diagonal translation
        // Translates the raindrop through 3D space by velocity * dt
        drop.pos += normFallDir * (drop.speed * dt);

        // 2. Ground collision detection or world bounds exit recycling
        if (drop.pos.y <= drop.groundHeight ||
            drop.pos.x < minX - 5.0f || drop.pos.x > maxX + 5.0f ||
            drop.pos.z < minZ - 5.0f || drop.pos.z > maxZ + 5.0f)
        {
            s_rainSeed = s_rainSeed * 1664525u + 1013904223u;
            float rand1 = (float)(s_rainSeed & 0x00FFFFFF) / (float)0x01000000;
            s_rainSeed = s_rainSeed * 1664525u + 1013904223u;
            float rand2 = (float)(s_rainSeed & 0x00FFFFFF) / (float)0x01000000;
            s_rainSeed = s_rainSeed * 1664525u + 1013904223u;
            float rand3 = (float)(s_rainSeed & 0x00FFFFFF) / (float)0x01000000;

            drop.pos.y = drop.resetHeight + rand1 * 5.0f;
            drop.pos.x = minX + rand2 * (maxX - minX);
            drop.pos.z = minZ + rand3 * (maxZ - minZ);

            // Re-evaluate ground/roof collision height for the new position
            drop.groundHeight = Rain::getGroundHeight(drop.pos.x, drop.pos.z);
        }
    }
}
