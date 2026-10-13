#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "DemoTourSystem.h"
#include "Camera.h"
#include "MotionSystem.h"
#include "objects/Boat.h"
#include "objects/HotAirBalloon.h"
#include "objects/AutumnArea.h"
#include "objects/Cabin.h"
#include "objects/Cave.h"
#include "objects/FirewoodProp.h"
#include "objects/Thunder.h"
#include "objects/Rain.h"
#include "texture/TextureManager.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <cstdio>

DemoTourSystem::DemoTourSystem()
    : m_active(false),
      m_paused(false),
      m_time(0.0f),
      m_duration(120.0f),
      m_currentSceneIndex(0),
      m_userYawOffset(0.0f),
      m_userPitchOffset(0.0f),
      m_userDollyOffset(0.0f),
      m_isUserDragging(false),
      m_lastDragTime(0.0f),
      m_sensitivity(0.12f),
      m_doorOpened(false),
      m_fireLit(false),
      m_trainHeadlightToggled(false),
      m_driverViewToggled(false),
      m_chaseViewToggled(false),
      m_rainActivated(false),
      m_strike1Triggered(false),
      m_strike2Triggered(false),
      m_torchEquipped(false),
      m_rayTracingToggled(false),
      m_lightingDemoToggled(false),
      m_textureDemoToggled(false),
      m_lastLoggedScene(-1),
      m_lastTitleUpdateTime(0.0f)
{
    initKeyframes();
    initScenes();
}

void DemoTourSystem::initScenes()
{
    m_scenes = {
        { 0, "Scene 1: Night Campsite & Blinn-Phong Illumination",    0.0f,  15.0f,
          "Campfire harmonic flames, twinkling starfield, moon corona, lake overlook & shadows" },
        { 1, "Scene 2: Wilderness Cabin & Fireplace Hearth",          15.0f,  30.0f,
          "Hinged door open, vintage interior exploration, firewood pickup & fireplace lighting" },
        { 2, "Scene 3: Lake Rowboat & Dynamic Gerstner Waves",        30.0f,  45.0f,
          "Dock waterfront, pilotable rowboat on Gerstner water displacement & rhythmic oars" },
        { 3, "Scene 4: Hot Air Balloon Flight & Sunset Transition",   45.0f,  60.0f,
          "Balloon ascent, burner lighting, transition from Night to Amber Sunset, valley vista" },
        { 4, "Scene 5: Lakeside Windmill, Shading & Textures",        60.0f,  75.0f,
          "Octagonal windmill rotation, Daytime transition, unlit mode & 8 photorealistic textures" },
        { 5, "Scene 6: Autumn Landscape & Tourist Train System",       75.0f,  92.0f,
          "Golden forest, 175m railway circuit, chase view, front pilot view & headlight spotlight" },
        { 6, "Scene 7: Dynamic Weather (Rain & Thunder Storm)",        92.0f, 106.0f,
          "Sky darkens, 18,000 continuous rain streaks with terrain collisions & thunder strikes" },
        { 7, "Scene 8: Subterranean Cave & GPU Ray Tracing",          106.0f, 120.0f,
          "Tunnel motes, flaming torch, treasure sanctum & multi-bounce GPU ray tracing on gold coins" }
    };
}

void DemoTourSystem::initKeyframes()
{
    // Smooth Catmull-Rom spline keyframe trajectory across all 8 feature scenes (120 seconds total)
    m_keyframes = {
        // --- SCENE 1: Night Campsite & Blinn-Phong Illumination (0s - 15s) ---
        {   0.0f, glm::vec3(-1.2f,  3.2f, 14.5f), glm::vec3(-3.8f, -0.2f,  4.5f) }, // Panoramic overview
        {   5.0f, glm::vec3(-2.2f,  1.8f, 10.0f), glm::vec3(-3.8f, -0.6f,  4.5f) }, // Gliding toward campfire
        {  10.0f, glm::vec3(-4.8f,  0.4f,  6.5f), glm::vec3(-3.8f, -0.5f,  4.5f) }, // Close-up flame tongues & sparks
        {  15.0f, glm::vec3(-7.0f,  0.7f,  5.0f), glm::vec3(-11.5f, 0.6f,  1.0f) }, // Panning toward western cabin

        // --- SCENE 2: Wilderness Cabin, Interior Furniture & Fireplace (15s - 30s) ---
        {  18.5f, glm::vec3(-10.8f, 0.65f,  2.8f), glm::vec3(-12.1f, 0.45f,  0.4f) }, // Approaching cabin porch & open doorway
        {  22.5f, glm::vec3(-12.1f, 0.40f,  0.2f), glm::vec3(-13.8f, 0.35f, -1.5f) }, // Entering interior: wrought-iron bed & furniture
        // {  26.5f, glm::vec3(-12.3f, 0.38f, -1.6f), glm::vec3(-10.82f,0.18f, -3.32f) }, // Facing stone fireplace, iron grate & open hearth
        {  30.0f, glm::vec3(-8.0f,  1.4f,   3.2f), glm::vec3( 2.0f, -0.5f,  7.5f) }, // Exiting toward lakeside dock

        // --- SCENE 3: Lake Rowboat & Dynamic Gerstner Waves (30s - 45s) ---
        {  34.0f, glm::vec3( 1.5f,  0.2f,  8.5f), glm::vec3( 3.8f, -1.0f,  8.0f) }, // Standing on wooden dock
        {  38.5f, glm::vec3( 3.2f, -0.4f,  7.0f), glm::vec3( 6.0f, -0.9f,  6.0f) }, // Tracking rowboat on Gerstner waves
        {  43.0f, glm::vec3( 6.5f,  0.2f,  4.5f), glm::vec3( 9.5f, -0.9f,  2.5f) }, // Open lake water surface & oars
        {  46.5f, glm::vec3( 9.5f,  2.2f,  3.5f), glm::vec3(12.0f,  5.2f,  2.0f) }, // Looking up at Hot Air Balloon

        // --- SCENE 4: Hot Air Balloon Flight & Sunset Transition (45s - 60s) ---
        {  50.5f, glm::vec3(11.0f,  6.5f,  6.0f), glm::vec3(12.0f,  7.5f,  1.0f) }, // Balloon climbing, Sunset glows
        {  55.5f, glm::vec3( 9.5f, 13.5f,  8.0f), glm::vec3( 6.0f,  3.0f,-12.0f) }, // High aerial golden hour vista
        {  60.0f, glm::vec3( 7.5f, 10.0f, -5.0f), glm::vec3( 7.5f,  1.8f,-25.0f) }, // Gliding toward northern windmill

        // --- SCENE 5: Lakeside Windmill, Shading & Textures (60s - 75s) ---
        {  64.0f, glm::vec3( 7.5f,  3.5f,-16.0f), glm::vec3( 7.5f,  2.0f,-27.5f) }, // Approaching windmill, Daytime
        {  68.0f, glm::vec3( 6.0f,  1.8f,-22.0f), glm::vec3( 7.5f,  2.2f,-27.5f) }, // Rotating sails, unlit shading demo
        {  72.0f, glm::vec3( 9.5f,  2.2f,-23.0f), glm::vec3( 7.5f,  2.0f,-27.5f) }, // Photorealistic textures demo
        {  75.5f, glm::vec3( 8.5f,  4.5f,-15.0f), glm::vec3(12.0f,  1.2f, 12.0f) }, // Flying south toward autumn area

        // --- SCENE 6: Autumn Landscape & Tourist Train System (75s - 92s) ---
        {  79.0f, glm::vec3(10.5f,  2.8f, 13.0f), glm::vec3(13.0f,  1.2f, 19.0f) }, // Autumn forest, dock & station
        {  83.0f, glm::vec3(11.5f,  1.8f, 18.0f), glm::vec3(14.0f,  1.1f, 22.0f) }, // Train Chase View & rotating wheels
        {  87.0f, glm::vec3(14.0f,  1.5f, 23.0f), glm::vec3(17.0f,  1.2f, 24.5f) }, // Front Pilot View & Spotlight
        {  91.5f, glm::vec3(18.5f,  2.5f, 18.0f), glm::vec3(12.0f,  1.2f,  8.0f) }, // Climbing as sky turns overcast

        // --- SCENE 7: Dynamic Weather (Rain & Thunder Storm) (92s - 106s) ---
        {  94.5f, glm::vec3(13.0f,  3.8f,  7.5f), glm::vec3( 0.0f,  1.0f, -4.0f) }, // 18,000 rain streaks in wind
        {  98.5f, glm::vec3( 7.5f,  2.5f,  2.5f), glm::vec3(-4.0f,  1.2f, -8.0f) }, // Thunder strike 1 & lightning bolt
        { 102.5f, glm::vec3(14.5f,  2.0f, -0.5f), glm::vec3(22.0f,  0.6f, -3.5f) }, // Thunder strike 2 across dark lake
        { 106.5f, glm::vec3(21.0f,  0.8f, -3.2f), glm::vec3(26.0f, -0.2f, -4.0f) }, // Approaching eastern cave portal

        // --- SCENE 8: Subterranean Cave & GPU Ray Tracing (106s - 120s) ---
        { 109.5f, glm::vec3(25.0f, -0.4f, -3.9f), glm::vec3(28.0f, -0.8f, -4.3f) }, // Tunnel motes & flaming torch
        { 113.0f, glm::vec3(28.0f, -0.75f,-4.3f), glm::vec3(30.5f, -0.95f,-4.6f) }, // Treasure Sanctum coins & chest
        { 116.0f, glm::vec3(29.8f, -0.85f,-4.4f), glm::vec3(30.6f, -0.95f,-4.6f) }, // Real-time GPU Ray Tracing ACTIVE
        { 118.0f, glm::vec3(29.5f, -0.8f, -4.3f), glm::vec3(30.6f, -0.95f,-4.6f) }, // Radiosity & contact crevice AO
        { 120.0f, glm::vec3(22.0f,  3.5f,  6.0f), glm::vec3( 5.0f,  0.0f,  0.0f) }  // Finale pullback overview of MoonLake
    };
}

void DemoTourSystem::setActive(bool active)
{
    m_active = active;
    if (m_active)
    {
        std::cout << "\n=======================================================\n";
        std::cout << ">>> 2-MINUTE AUTOMATIC CINEMATIC FEATURE TOUR ACTIVE <<<\n";
        std::cout << "  - Automatically moves through all 8 scenes & features in 120s\n";
        std::cout << "  - [Mouse Drag]        : Free 360-degree interactive camera look\n";
        std::cout << "  - [Scroll Wheel]      : Dolly / Zoom in & out\n";
        std::cout << "  - [SPACE] / [P]       : Pause / Resume playback\n";
        std::cout << "  - [1] - [8]           : Jump directly to Scenes 1 through 8\n";
        std::cout << "  - [F1]                : Disengage tour & return to manual flight\n";
        std::cout << "=======================================================\n\n";
    }
    else
    {
        std::cout << "[Demo Tour] Stopped. Returned to manual free-look flight.\n";
    }
}

void DemoTourSystem::toggleActive()
{
    setActive(!m_active);
}

void DemoTourSystem::togglePause()
{
    m_paused = !m_paused;
    std::cout << "[Demo Tour] " << (m_paused ? "PAUSED. (Mouse look remains active!)" : "RESUMED.") << std::endl;
}

void DemoTourSystem::setTime(float t)
{
    m_time = glm::clamp(t, 0.0f, m_duration);
}

void DemoTourSystem::jumpToScene(int sceneIndex)
{
    if (sceneIndex >= 0 && sceneIndex < static_cast<int>(m_scenes.size()))
    {
        m_time = m_scenes[sceneIndex].startTime;
        m_currentSceneIndex = sceneIndex;
        m_userYawOffset = 0.0f;
        m_userPitchOffset = 0.0f;
        m_userDollyOffset = 0.0f;
        std::cout << "\n[Demo Tour] >> Jumped to Scene " << (sceneIndex + 1)
                  << ": " << m_scenes[sceneIndex].name << " (" << m_scenes[sceneIndex].description << ")\n\n";
    }
}

void DemoTourSystem::processMouseMovement(float xoffset, float yoffset, bool isInteracting)
{
    m_isUserDragging = isInteracting;
    if (isInteracting)
    {
        m_userYawOffset   += xoffset * m_sensitivity;
        m_userPitchOffset += yoffset * m_sensitivity;
        m_lastDragTime = m_time;
    }
}

void DemoTourSystem::processMouseScroll(float yoffset)
{
    m_userDollyOffset += yoffset * 0.8f;
    m_lastDragTime = m_time;
}

glm::vec3 DemoTourSystem::catmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                     const glm::vec3& p2, const glm::vec3& p3, float u)
{
    float u2 = u * u;
    float u3 = u2 * u;
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * u +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
}

void DemoTourSystem::evaluatePath(float t, glm::vec3& outPos, glm::vec3& outTarget) const
{
    float clampedTime = glm::clamp(t, 0.0f, m_duration);
    int n = static_cast<int>(m_keyframes.size());
    if (n == 0) return;
    if (n == 1)
    {
        outPos = m_keyframes[0].pos;
        outTarget = m_keyframes[0].target;
        return;
    }

    // Find the current keyframe span
    int idx = 0;
    while (idx < n - 2 && m_keyframes[idx + 1].time < clampedTime)
    {
        idx++;
    }

    float t0 = m_keyframes[idx].time;
    float t1 = m_keyframes[idx + 1].time;
    float span = t1 - t0;
    float u = (span > 0.0001f) ? glm::clamp((clampedTime - t0) / span, 0.0f, 1.0f) : 0.0f;

    int i0 = (std::max)(0, idx - 1);
    int i1 = idx;
    int i2 = idx + 1;
    int i3 = (std::min)(n - 1, idx + 2);

    outPos    = catmullRom(m_keyframes[i0].pos, m_keyframes[i1].pos, m_keyframes[i2].pos, m_keyframes[i3].pos, u);
    outTarget = catmullRom(m_keyframes[i0].target, m_keyframes[i1].target, m_keyframes[i2].target, m_keyframes[i3].target, u);
}

void DemoTourSystem::syncStateForTime(float t,
                                      Cabin& cabin,
                                      AutumnArea& autumnArea,
                                      Cave& cave,
                                      bool& rainEnabled,
                                      TimeOfDay& currentTime,
                                      bool& lightingDisabled)
{
    // Door state
    m_doorOpened = (t >= 17.5f && t < 31.0f);
    cabin.setDoorOpen(m_doorOpened);

    // Fireplace state
    m_fireLit = (t >= 22.0f);
    cabin.setFireLit(m_fireLit);

    // Rain & weather state
    rainEnabled = (t >= 93.0f && t < 106.0f);

    // Torch & ray tracing state
    cave.setPlayerCarryingTorch(t >= 107.0f);
    cave.setCoinRayTracingEnabled(t >= 113.0f && t <= 120.0f && !(t >= 116.0f && t < 117.5f));

    // Time of day
    if (t < 48.0f)
        currentTime = TIME_NIGHT;
    else if (t < 63.0f)
        currentTime = TIME_SUNSET;
    else if (t < 92.0f)
        currentTime = TIME_DAY;
    else
        currentTime = TIME_NIGHT;

    // Lighting demo state
    lightingDisabled = (t >= 68.0f && t < 70.5f);
}

void DemoTourSystem::update(float dt,
                            GLFWwindow* window,
                            Camera& camera,
                            Boat& boat,
                            HotAirBalloon& balloon,
                            AutumnArea& autumnArea,
                            Cabin& cabin,
                            FirewoodManager& firewood,
                            Cave& cave,
                            Thunder& thunder,
                            Rain& rain,
                            bool& rainEnabled,
                            TimeOfDay& currentTime,
                            bool& lightingDisabled)
{
    if (!m_active) return;

    // 1. Advance playback timeline
    if (!m_paused)
    {
        m_time += dt;
        if (m_time > m_duration)
        {
            m_time = 0.0f; // Loop tour seamlessly
            m_doorOpened = false;
            m_fireLit = false;
            m_trainHeadlightToggled = false;
            m_driverViewToggled = false;
            m_chaseViewToggled = false;
            m_strike1Triggered = false;
            m_strike2Triggered = false;
            m_torchEquipped = false;
            m_rayTracingToggled = false;
        }
    }

    // 2. Identify active scene
    m_currentSceneIndex = 0;
    for (size_t i = 0; i < m_scenes.size(); i++)
    {
        if (m_time >= m_scenes[i].startTime && m_time < m_scenes[i].endTime)
        {
            m_currentSceneIndex = static_cast<int>(i);
            break;
        }
    }

    // Log scene transition to console when scene changes
    if (m_currentSceneIndex != m_lastLoggedScene)
    {
        m_lastLoggedScene = m_currentSceneIndex;
        std::cout << "\n-------------------------------------------------------\n";
        std::cout << "[TOUR " << std::setfill('0') << std::setw(2) << static_cast<int>(m_time) / 60 << ":"
                  << std::setw(2) << static_cast<int>(m_time) % 60 << " / 02:00] "
                  << m_scenes[m_currentSceneIndex].name << "\n";
        std::cout << "  Feature: " << m_scenes[m_currentSceneIndex].description << "\n";
        std::cout << "-------------------------------------------------------\n";
    }

    // 3. Actuate Automated Features along the timeline
    // --- Scene 1: Night Campsite & Blinn-Phong Illumination (0s - 15s) ---
    if (m_time < 15.0f)
    {
        currentTime = TIME_NIGHT;
        rainEnabled = false;
        lightingDisabled = false;
        TextureManager::getInstance().setEnabled(true);
        cave.setPlayerCarryingTorch(false);
        cave.setCoinRayTracingEnabled(false);
    }
    // --- Scene 2: Wilderness Cabin & Fireplace Hearth (15s - 30s) ---
    else if (m_time < 30.0f)
    {
        if (m_time >= 17.5f && !m_doorOpened)
        {
            cabin.setDoorOpen(true);
            m_doorOpened = true;
            std::cout << "[Tour Action] Cabin front door swung smoothly open on its hinge!\n";
        }
        if (m_time >= 22.0f && !m_fireLit)
        {
            // cabin.setFireLit(true);
            cabin.addWood();
            cabin.addWood();   
            m_fireLit = true;
            std::cout << "[Tour Action] Gathered firewood fed into arched hearth! Cabin fireplace ignited & chimney smoking.\n";
        }
    }
    // --- Scene 3: Lake Rowboat & Dynamic Gerstner Waves (30s - 45s) ---
    else if (m_time < 45.0f)
    {
        // Drive the rowboat forward across the lake water basin
        if (m_time >= 34.0f)
        {
            boat.processInput(true, false, false, false, dt);
        }
    }
    // --- Scene 4: Hot Air Balloon Flight & Sunset Transition (45s - 60s) ---
    else if (m_time < 60.0f)
    {
        // Burner boost & ascent
        balloon.processInput(true, false, false, false, true, false, true, dt);

        if (m_time >= 48.0f)
        {
            currentTime = TIME_SUNSET;
        }
    }
    // --- Scene 5: Lakeside Windmill, Shading & Textures (60s - 75s) ---
    else if (m_time < 75.0f)
    {
        if (m_time >= 63.0f)
        {
            currentTime = TIME_DAY;
        }

        // Feature demonstration: Unlit base colors vs Blinn-Phong shading
        if (m_time >= 68.0f && m_time < 70.5f)
        {
            if (!lightingDisabled)
            {
                lightingDisabled = true;
                std::cout << "[Tour Action] Shading toggle: UNLIT pure base colors displayed for technical verification.\n";
            }
        }
        else
        {
            if (lightingDisabled)
            {
                lightingDisabled = false;
                std::cout << "[Tour Action] Restored full Blinn-Phong illumination.\n";
            }
        }

        // Feature demonstration: 8 Photorealistic Material Maps toggle
        if (m_time >= 71.2f && m_time < 73.2f)
        {
            if (TextureManager::getInstance().isEnabled())
            {
                TextureManager::getInstance().setEnabled(false);
                std::cout << "[Tour Action] Textures toggle: Base shaded clay colors displayed.\n";
            }
        }
        else
        {
            if (!TextureManager::getInstance().isEnabled())
            {
                TextureManager::getInstance().setEnabled(true);
                std::cout << "[Tour Action] Restored 8 photorealistic material texture maps.\n";
            }
        }
    }
    // --- Scene 6: Autumn Landscape & Tourist Train System (75s - 92s) ---
    else if (m_time < 92.0f)
    {
        // Ensure tourist train accelerates smoothly along the 175-meter Catmull-Rom spline railway track
        autumnArea.processTrainInput(true, false, false, dt);

        // Switch to Driver's Front Pilot View at 84s
        if (m_time >= 84.0f && m_time < 89.0f)
        {
            if (!m_driverViewToggled && autumnArea.getTrainCameraViewMode() == 0)
            {
                autumnArea.toggleTrainCameraView();
                m_driverViewToggled = true;
                std::cout << "[Tour Action] Train Camera switched to: Engineer Front Pilot View!\n";
            }
        }
        else if (m_time >= 89.0f)
        {
            if (!m_chaseViewToggled && autumnArea.getTrainCameraViewMode() == 1)
            {
                autumnArea.toggleTrainCameraView();
                m_chaseViewToggled = true;
                std::cout << "[Tour Action] Train Camera switched to: Scenic Chase View!\n";
            }
        }

        // Turn ON Locomotive Headlight Spotlight beam at 86.5s
        if (m_time >= 86.5f && !m_trainHeadlightToggled)
        {
            if (!autumnArea.isTrainHeadlightOn())
            {
                autumnArea.toggleTrainHeadlight();
                m_trainHeadlightToggled = true;
                std::cout << "[Tour Action] Locomotive Headlight Spotlight beam turned ON!\n";
            }
        }
    }
    // --- Scene 7: Dynamic Weather (Rain & Thunder Storm) (92s - 106s) ---
    else if (m_time < 106.0f)
    {
        if (m_time >= 93.0f)
        {
            currentTime = TIME_NIGHT;
            rainEnabled = true;
        }

        // Thunder strike 1
        if (m_time >= 97.5f && !m_strike1Triggered)
        {
            thunder.triggerStrike();
            m_strike1Triggered = true;
            std::cout << "[Tour Action] Sky Thunder Strike 1 triggered! Lightning flash & 3D branching bolt illuminated.\n";
        }

        // Thunder strike 2
        if (m_time >= 101.5f && !m_strike2Triggered)
        {
            thunder.triggerStrike();
            m_strike2Triggered = true;
            std::cout << "[Tour Action] Sky Thunder Strike 2 triggered across the storm-swept lake!\n";
        }
    }
    // --- Scene 8: Subterranean Cave & GPU Ray Tracing (106s - 120s) ---
    else
    {
        rainEnabled = false;

        // Equip flaming torch for dark cave exploration
        if (m_time >= 107.5f && !m_torchEquipped)
        {
            cave.setPlayerCarryingTorch(true);
            m_torchEquipped = true;
            std::cout << "[Tour Action] Flaming torch taken into hand to illuminate dark tunnel & motes.\n";
        }

        // Real-Time Analytical GPU Ray Tracing ON at 112.5s
        if (m_time >= 112.5f && !m_rayTracingToggled)
        {
            cave.setCoinRayTracingEnabled(true);
            m_rayTracingToggled = true;
            std::cout << "[Tour Action] Real-Time Analytical GPU Ray Tracing: ACTIVE on gold coins!\n";
            std::cout << "             Multi-bounce metallic reflection radiosity & contact crevice AO.\n";
        }

        // Side-by-side comparison: briefly toggle rasterization at 116s, then back to ray tracing at 117.5s
        if (m_time >= 116.0f && m_time < 117.5f)
        {
            if (cave.isCoinRayTracingEnabled())
            {
                cave.setCoinRayTracingEnabled(false);
                std::cout << "[Tour Action] Ray Tracing OFF (Rasterized Blinn-Phong baseline for comparison).\n";
            }
        }
        else if (m_time >= 117.5f)
        {
            if (!cave.isCoinRayTracingEnabled())
            {
                cave.setCoinRayTracingEnabled(true);
                std::cout << "[Tour Action] Ray Tracing restored (Conductor Fresnel F0 active).\n";
            }
        }
    }

    // 4. Evaluate automated camera trajectory on Catmull-Rom spline
    glm::vec3 autoPos, autoTarget;
    evaluatePath(m_time, autoPos, autoTarget);

    glm::vec3 dir = glm::normalize(autoTarget - autoPos);
    float basePitch = glm::degrees(std::asin(glm::clamp(dir.y, -0.999f, 0.999f)));
    float baseYaw   = glm::degrees(std::atan2(dir.z, dir.x));

    // 5. Gently damp user mouse look offsets back toward director center when user releases mouse
    if (!m_isUserDragging && (m_time - m_lastDragTime > 0.8f))
    {
        float decay = std::exp(-2.2f * dt);
        m_userYawOffset   *= decay;
        m_userPitchOffset *= decay;
        m_userDollyOffset *= decay;
        if (std::abs(m_userYawOffset) < 0.02f)   m_userYawOffset = 0.0f;
        if (std::abs(m_userPitchOffset) < 0.02f) m_userPitchOffset = 0.0f;
        if (std::abs(m_userDollyOffset) < 0.02f) m_userDollyOffset = 0.0f;
    }

    // 6. Combine automated director trajectory with interactive mouse look & zoom
    float finalYaw   = baseYaw + m_userYawOffset;
    float finalPitch = glm::clamp(basePitch + m_userPitchOffset, -89.0f, 89.0f);

    glm::vec3 finalPos = autoPos + camera.front * m_userDollyOffset;
    camera.position = finalPos;
    camera.setOrientation(finalYaw, finalPitch);

    // 7. Update window title with feature description, timestamp, and controls for video recording
    float curTimeSec = static_cast<float>(glfwGetTime());
    if (curTimeSec - m_lastTitleUpdateTime > 0.15f)
    {
        m_lastTitleUpdateTime = curTimeSec;
        int mins = static_cast<int>(m_time) / 60;
        int secs = static_cast<int>(m_time) % 60;
        char buffer[320];
        std::snprintf(buffer, sizeof(buffer),
            "MoonLake [%s] [%02d:%02d / 02:00] %s | Mouse Look: %s [F1: Tour, SPACE: Pause, 1-8: Jump]",
            m_paused ? "PAUSED" : "RECORDING",
            mins, secs,
            m_scenes[m_currentSceneIndex].name.c_str(),
            (m_isUserDragging || std::abs(m_userYawOffset) > 0.5f || std::abs(m_userPitchOffset) > 0.5f) ? "ACTIVE" : "DIRECTOR");
        glfwSetWindowTitle(window, buffer);
    }
}

std::string DemoTourSystem::getCurrentSceneTitle() const
{
    if (m_currentSceneIndex >= 0 && m_currentSceneIndex < static_cast<int>(m_scenes.size()))
    {
        return m_scenes[m_currentSceneIndex].name;
    }
    return "MoonLake Showcase";
}

std::string DemoTourSystem::getCurrentSceneDescription() const
{
    if (m_currentSceneIndex >= 0 && m_currentSceneIndex < static_cast<int>(m_scenes.size()))
    {
        return m_scenes[m_currentSceneIndex].description;
    }
    return "";
}
