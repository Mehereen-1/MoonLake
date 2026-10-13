#ifndef DEMO_TOUR_SYSTEM_H
#define DEMO_TOUR_SYSTEM_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

// Forward declarations
struct GLFWwindow;
class Camera;
class Boat;
class HotAirBalloon;
class AutumnArea;
class Cabin;
class FirewoodManager;
class Cave;
class Thunder;
class Rain;
enum TimeOfDay;

// ---------------------------------------------------------------------------
// TourKeyframe: Position & Target waypoint along the 120-second timeline
// ---------------------------------------------------------------------------
struct TourKeyframe {
    float time;
    glm::vec3 pos;
    glm::vec3 target;
};

// ---------------------------------------------------------------------------
// SceneInfo: Metadata for each of the 8 feature scenes
// ---------------------------------------------------------------------------
struct SceneInfo {
    int id;
    std::string name;
    float startTime;
    float endTime;
    std::string description;
};

// ---------------------------------------------------------------------------
// DemoTourSystem
// Orchestrates a complete 2-minute (120-second) cinematic feature demonstration.
// Seamlessly navigates all 8 major feature areas, actuates interactive objects,
// transitions environments, and allows simultaneous 360-degree interactive mouse
// look and zoom without breaking or canceling the automated travel.
// ---------------------------------------------------------------------------
class DemoTourSystem {
public:
    DemoTourSystem();

    // Toggle tour mode active state
    void setActive(bool active);
    bool isActive() const { return m_active; }
    void toggleActive();

    // Pause / Resume playback
    void setPaused(bool paused) { m_paused = paused; }
    bool isPaused() const { return m_paused; }
    void togglePause();

    // Time & Scene Scrubbing
    void setTime(float t);
    float getTime() const { return m_time; }
    float getDuration() const { return m_duration; }
    int getCurrentSceneIndex() const { return m_currentSceneIndex; }
    void jumpToScene(int sceneIndex);

    // Mouse Interaction (Called from GLFW mouse & scroll callbacks)
    void processMouseMovement(float xoffset, float yoffset, bool isInteracting);
    void processMouseScroll(float yoffset);

    // Frame Update: computes smooth camera trajectory, actuates automated features,
    // applies interactive mouse look offsets, and updates the window title.
    void update(float dt,
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
                bool& lightingDisabled);

    // Information helpers
    const std::vector<SceneInfo>& getScenes() const { return m_scenes; }
    std::string getCurrentSceneTitle() const;
    std::string getCurrentSceneDescription() const;

private:
    void initKeyframes();
    void initScenes();
    void syncStateForTime(float t,
                          Cabin& cabin,
                          AutumnArea& autumnArea,
                          Cave& cave,
                          bool& rainEnabled,
                          TimeOfDay& currentTime,
                          bool& lightingDisabled);

    // Spline evaluation
    void evaluatePath(float t, glm::vec3& outPos, glm::vec3& outTarget) const;
    static glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                const glm::vec3& p2, const glm::vec3& p3, float u);

    bool m_active;
    bool m_paused;
    float m_time;
    float m_duration;
    int m_currentSceneIndex;

    // Interactive mouse look offsets
    float m_userYawOffset;
    float m_userPitchOffset;
    float m_userDollyOffset;
    bool m_isUserDragging;
    float m_lastDragTime;
    float m_sensitivity;

    // Keyframes and scene metadata
    std::vector<TourKeyframe> m_keyframes;
    std::vector<SceneInfo> m_scenes;

    // Feature action tracking flags (to trigger once per pass)
    bool m_doorOpened;
    bool m_fireLit;
    bool m_trainHeadlightToggled;
    bool m_driverViewToggled;
    bool m_chaseViewToggled;
    bool m_rainActivated;
    bool m_strike1Triggered;
    bool m_strike2Triggered;
    bool m_torchEquipped;
    bool m_rayTracingToggled;
    bool m_lightingDemoToggled;
    bool m_textureDemoToggled;
    int m_lastLoggedScene;
    float m_lastTitleUpdateTime;
};

#endif // DEMO_TOUR_SYSTEM_H
