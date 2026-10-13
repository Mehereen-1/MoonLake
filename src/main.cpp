#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "MotionSystem.h"
// Unit Geometric Primitives
#include "primitives/Cube.h"
#include "primitives/Pyramid.h"
#include "primitives/TriangularPrism.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"
#include "primitives/Plane.h"
#include "primitives/Rock.h"
#include "primitives/GlowQuad.h"
#include "primitives/Branch.h"

// Composite Objects & Landscape Systems
#include "objects/Stars.h"
#include "objects/Cloud.h"
#include "objects/Mountain.h"
#include "objects/Water.h"
#include "objects/GrassLines.h"
#include "objects/TerrainMesh.h"
#include "objects/Cabin.h"
#include "objects/Cave.h"
#include "objects/PineTree.h"
#include "objects/Campfire.h"
#include "objects/Chair.h"
#include "objects/Windmill.h"
#include "objects/Rain.h"
#include "objects/Thunder.h"
#include "objects/Boat.h"
#include "objects/HotAirBalloon.h"
#include "objects/AutumnArea.h"
#include "objects/FirewoodProp.h"
#include "texture/TextureManager.h"
#include "DemoTourSystem.h"

#include <iostream>
#include <vector>

// Window configuration
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera instance (Vantage point overlooking expanded lake, campsite, dock, and distant mountains)
Camera camera(glm::vec3(-1.2f, 2.9f, 13.8f), glm::vec3(0.0f, 1.0f, 0.0f), -87.0f, -7.0f);

// 2-Minute Cinematic Feature Showcase Tour System
DemoTourSystem demoTour;

// Mouse state
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorLocked = false;
bool isDragging = false;
bool caveTourActive = false;
float caveTourProgress = 0.0f;
bool cKeyPressed = false;
bool boatModeActive = false;
bool bKeyPressed = false;
bool balloonModeActive = false;
bool hKeyPressed = false;
bool trainRideActive = false;
bool uKeyPressed = false;
bool xKeyPressed = false;
bool pKeyPressed = false;
bool zKeyPressed = false;
bool mKeyPressed = false;
bool eKeyPressed = false;
bool fKeyPressed = false;
bool oKeyPressed = false;
bool jKeyPressed = false;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, float dt, Boat& boat, HotAirBalloon& balloon, AutumnArea& autumnArea, Cabin& cabin, FirewoodManager& firewood, Cave& cave, Thunder& thunder);

// ---------------------------------------------------------------------------
// Time of Day Presets (Enum defined in MotionSystem.h)
// ---------------------------------------------------------------------------

// Preset 1: Night (Luminous Teal-Indigo Sky matching Image 1)
const EnvironmentPreset NIGHT_PRESET = {
    glm::vec3(0.08f, 0.22f, 0.34f),   // Luminous teal/cyan horizon haze
    glm::vec3(0.012f, 0.030f, 0.08f), // Deep indigo navy zenith
    glm::vec3(0.012f, 0.030f, 0.08f), // Deep night background
    glm::vec3(20.0f, -18.0f, -35.0f), // Sun position (set deep below horizon)
    glm::vec3(-10.0f, 22.0f, -35.0f), // Moon position (Elevated majestically in the night sky)
    glm::vec3(-10.0f, 22.0f, -35.0f), // Dominant celestial light position
    glm::vec3(0.52f, 0.68f, 0.88f),   // Soft silvery-blue moonlight
    glm::vec3(0.05f, 0.08f, 0.14f),   // Dark midnight ambient
    glm::vec3(0.70f, 0.80f, 0.92f),   // Moonlit cloud tops (silvery celestial glow)
    glm::vec3(0.24f, 0.30f, 0.40f),   // Cloud undersides (deep alpine night ash)
    glm::vec3(0.03f, 0.08f, 0.16f),   // Deep nighttime lake water
    1.0f,                             // Stars fully visible
    1.0f,                             // 100% Moon dominant
    0.0f,                             // Sun Alpha (hidden)
    1.0f,                             // Moon Alpha (visible)
    0.0f                              // Sunset Factor (0 = Night)
};

// Preset 2: Sunset / Golden Hour (Warm Amber Sky matching Image 2)
const EnvironmentPreset SUNSET_PRESET = {
    glm::vec3(0.92f, 0.48f, 0.18f),   // Warm glowing orange/peach horizon
    glm::vec3(0.18f, 0.22f, 0.44f),   // Dusk purple-blue upper sky
    glm::vec3(0.18f, 0.22f, 0.44f),   // Twilight background
    glm::vec3(18.0f, 13.5f, -38.0f),  // Sun position (Golden Sun resting low over horizon)
    glm::vec3(-24.0f, -14.0f, -35.0f),// Moon position (set below horizon on opposite side)
    glm::vec3(18.0f, 13.5f, -38.0f),  // Dominant celestial light position
    glm::vec3(1.0f, 0.68f, 0.32f),    // Rich golden sunset sunlight
    glm::vec3(0.24f, 0.18f, 0.15f),   // Warm dusk ambient
    glm::vec3(0.98f, 0.74f, 0.48f),   // Sunset cloud tops (golden peach / amber sunlit rim)
    glm::vec3(0.50f, 0.38f, 0.46f),   // Cloud undersides (dusk lavender-ash ambient shadow)
    glm::vec3(0.18f, 0.14f, 0.18f),   // Sunset reflective water
    0.20f,                            // Subtle faint stars appearing
    0.0f,                             // 100% Sun dominant
    1.0f,                             // Sun Alpha (visible)
    0.0f,                             // Moon Alpha (hidden)
    1.0f                              // Sunset Factor (1 = Sunset)
};

// Preset 3: Day (Radiant Azure Blue Sky & Bright Sunlit Terrain)
const EnvironmentPreset DAY_PRESET = {
    glm::vec3(0.65f, 0.82f, 0.96f),   // Soft cyan/white atmospheric horizon
    glm::vec3(0.15f, 0.45f, 0.85f),   // Rich azure daytime sky
    glm::vec3(0.15f, 0.45f, 0.85f),   // Sky background
    glm::vec3(12.0f, 28.0f, -32.0f),  // Sun position (Bright high midday Sun)
    glm::vec3(-24.0f, -18.0f, -35.0f),// Moon position (set deep below horizon)
    glm::vec3(12.0f, 28.0f, -32.0f),  // Dominant celestial light position
    glm::vec3(1.15f, 1.12f, 1.05f),   // Radiant warm crisp sunlight
    glm::vec3(0.52f, 0.56f, 0.62f),   // Bright vibrant daytime sky ambient
    glm::vec3(0.96f, 0.97f, 0.99f),   // Day cloud tops (crisp pure white cumulus)
    glm::vec3(0.66f, 0.70f, 0.76f),   // Cloud undersides (soft sky-ambient ash shadow)
    glm::vec3(0.08f, 0.22f, 0.36f),   // Clear blue daytime water
    0.0f,                             // Stars invisible
    0.0f,                             // 100% Sun dominant
    1.0f,                             // Sun Alpha (visible)
    0.0f,                             // Moon Alpha (hidden)
    0.0f                              // Sunset Factor (0 = Day)
};

TimeOfDay currentTime = TIME_NIGHT;
bool rainEnabled = false; // By default not raining
bool lightingDisabled = false; // Set to true to disable all lighting and display pure unlit base colors

// ---------------------------------------------------------------------------
// Prop Instance Structure (Rocks, branches, etc.)
// ---------------------------------------------------------------------------
struct PropInstance {
    glm::vec3 pos;
    glm::vec3 scale;
    float rotY = 0.0f;
};

int main(int argc, char* argv[])
{
    int maxTestFrames = -1;
    // Check command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--tour" || arg == "--demo" || arg == "-t")
        {
            demoTour.setActive(true);
        }
        else if (arg == "--test" || arg == "--benchmark")
        {
            maxTestFrames = 60;
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                maxTestFrames = std::atoi(argv[++i]);
            }
        }
    }

    // ------------------------------------------------------------------
    // 1. Initialize GLFW & create window
    // ------------------------------------------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA antialiasing

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MoonLake - 3D Camping Scene (WASD + Mouse Flight)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Register GLFW Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // ------------------------------------------------------------------
    // 2. Initialize GLAD
    // ------------------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer:       " << glGetString(GL_RENDERER) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // ------------------------------------------------------------------
    // 3. Shaders
    // ------------------------------------------------------------------
    Shader mainShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Shader skyShader("shaders/sky_vertex.glsl", "shaders/sky_fragment.glsl");
    Shader starsShader("shaders/stars_vertex.glsl", "shaders/stars_fragment.glsl");
    Shader waterShader("shaders/water_vertex.glsl", "shaders/water_fragment.glsl");
    Shader rainShader("shaders/rain_vertex.glsl", "shaders/rain_fragment.glsl");
    Shader shadowShader("shaders/shadow_vertex.glsl", "shaders/shadow_fragment.glsl");

    // ------------------------------------------------------------------
    // 3b. Initialize Realistic Texture Library (8 High-Detail Material Maps)
    // ------------------------------------------------------------------
    TextureManager::getInstance().init("textures");

    // ------------------------------------------------------------------
    // 3c. Dynamic Directional Shadow Mapping (2048x2048 Depth FBO)
    // ------------------------------------------------------------------
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float shadowBorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, shadowBorderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------------
    // ------------------------------------------------------------------
    // 4. Create Scene Objects
    // ------------------------------------------------------------------
    // 360-degree Atmospheric Sky Dome (Full panoramic horizon and zenith in all directions)
    Sphere skyDome(160.0f, 40, 22, glm::vec3(1.0f));

    // Twinkling Starfield (650 stars distributed high across the expanded dome)
    Stars stars(650);

    // Celestial Bodies (Moon & Sun)
    Sphere moon(2.2f, 48, 24, glm::vec3(0.96f, 0.97f, 0.92f));
    moon.setRotation(18.0f, glm::vec3(0.3f, 1.0f, 0.2f));
    GlowQuad moonGlow(13.0f);

    Sphere sun(2.5f, 48, 24, glm::vec3(1.0f, 0.95f, 0.85f));
    GlowQuad sunGlow(16.0f);

    // 3D Clouds (Hierarchical modeling drifting across the wide sky)
    Sphere cloudPuffSphere(1.0f, 24, 14, glm::vec3(1.0f));
    std::vector<Cloud> clouds = {
        Cloud(glm::vec3(-35.0f, 14.5f, -32.0f), glm::vec3(2.6f, 1.6f, 1.6f), 0.32f, 0),
        Cloud(glm::vec3(-18.0f, 11.5f, -24.0f), glm::vec3(2.2f, 1.4f, 1.4f), 0.28f, 1),
        Cloud(glm::vec3(  3.5f, 16.8f, -36.0f), glm::vec3(3.0f, 1.8f, 1.8f), 0.36f, 0),
        Cloud(glm::vec3( 25.0f, 12.2f, -28.0f), glm::vec3(2.4f, 1.5f, 1.5f), 0.25f, 2),
        Cloud(glm::vec3( 42.0f, 15.0f, -34.0f), glm::vec3(2.8f, 1.6f, 1.7f), 0.30f, 1),
        Cloud(glm::vec3(-48.0f, 10.5f, -18.0f), glm::vec3(2.1f, 1.3f, 1.4f), 0.26f, 2),
        Cloud(glm::vec3( -8.5f, 17.5f, -42.0f), glm::vec3(3.2f, 1.9f, 1.8f), 0.38f, 0),
        Cloud(glm::vec3( 15.0f,  9.8f, -18.0f), glm::vec3(1.9f, 1.2f, 1.3f), 0.22f, 2),
        Cloud(glm::vec3(-28.0f, 13.0f,  -8.0f), glm::vec3(2.3f, 1.4f, 1.4f), 0.27f, 1)
    };

    // ------------------------------------------------------------------
    // Distant Alpine Mountain Ranges (Panoramic Natural Valley Amphitheater)
    // ------------------------------------------------------------------
    Mountain mountainRidgeNorth(130.0f, 38.0f, 35.0f);
    mountainRidgeNorth.setPosition(glm::vec3(5.0f, 1.0f, -62.0f));

    Mountain mountainRidgeNW(105.0f, 32.0f, 30.0f);
    mountainRidgeNW.setPosition(glm::vec3(-42.0f, 0.5f, -50.0f));

    Mountain mountainRidgeNE(100.0f, 30.0f, 28.0f);
    mountainRidgeNE.setPosition(glm::vec3(45.0f, 0.8f, -48.0f));

    Mountain mountainRidgeWest(85.0f, 22.0f, 26.0f);
    mountainRidgeWest.setPosition(glm::vec3(-58.0f, 0.0f, -12.0f));

    Mountain mountainRidgeEast(75.0f, 24.0f, 26.0f);
    mountainRidgeEast.setPosition(glm::vec3(105.0f, 0.5f, -10.0f));

    // ------------------------------------------------------------------
    // 1. ORGANIC CONTINUOUS TERRAIN MESH & WATER BASIN
    // ------------------------------------------------------------------
    // High-resolution subdivided organic terrain covering the entire world (-85m to +85m, -60m to +130m)
    // with smooth slopes, analytical lakebed, campsite lawn, rolling bluffs, foothills, and distant southern ridge
    TerrainMesh worldTerrain(-85.0f, 85.0f, -60.0f, 130.0f, 170, 190);

    // High-resolution subdivided lake water surface covering the expanded lake basin
    Water lake(50.0f, 70.0f, 80, 92);
    lake.setPosition(glm::vec3(12.0f, -1.18f, -4.0f));

    // ------------------------------------------------------------------
    // 4. Wooden Dock & Rowboat in the Lake (Waterfront Foreground)
    // ------------------------------------------------------------------
    struct BoxMeshInstance {
        glm::vec3 pos;
        glm::vec3 scale;
        glm::vec3 color;
        float rotY = 0.0f;
    };

    std::vector<BoxMeshInstance> dockPieces = {
        // Dock vertical support posts into the water
        { glm::vec3(0.2f, -1.05f, 6.8f), glm::vec3(0.18f, 1.2f, 0.18f), glm::vec3(0.24f, 0.17f, 0.12f) },
        { glm::vec3(0.2f, -1.05f, 9.2f), glm::vec3(0.18f, 1.2f, 0.18f), glm::vec3(0.24f, 0.17f, 0.12f) },
        { glm::vec3(2.4f, -1.05f, 6.8f), glm::vec3(0.18f, 1.2f, 0.18f), glm::vec3(0.24f, 0.17f, 0.12f) },
        { glm::vec3(2.4f, -1.05f, 9.2f), glm::vec3(0.18f, 1.2f, 0.18f), glm::vec3(0.24f, 0.17f, 0.12f) },
        // Dock horizontal stringer beams
        { glm::vec3(1.3f, -0.60f, 6.85f), glm::vec3(2.4f, 0.14f, 0.16f), glm::vec3(0.30f, 0.20f, 0.14f) },
        { glm::vec3(1.3f, -0.60f, 9.15f), glm::vec3(2.4f, 0.14f, 0.16f), glm::vec3(0.30f, 0.20f, 0.14f) },
        // Wooden dock deck planks
        { glm::vec3(1.3f, -0.52f, 6.4f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.40f, 0.29f, 0.19f) },
        { glm::vec3(1.3f, -0.52f, 6.8f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.44f, 0.31f, 0.20f) },
        { glm::vec3(1.3f, -0.52f, 7.2f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.38f, 0.27f, 0.18f) },
        { glm::vec3(1.3f, -0.52f, 7.6f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.42f, 0.30f, 0.20f) },
        { glm::vec3(1.3f, -0.52f, 8.0f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.39f, 0.28f, 0.18f) },
        { glm::vec3(1.3f, -0.52f, 8.4f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.43f, 0.31f, 0.21f) },
        { glm::vec3(1.3f, -0.52f, 8.8f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.41f, 0.29f, 0.19f) },
        { glm::vec3(1.3f, -0.52f, 9.2f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.40f, 0.28f, 0.18f) },
        { glm::vec3(1.3f, -0.52f, 9.6f), glm::vec3(2.5f, 0.07f, 0.32f), glm::vec3(0.42f, 0.30f, 0.20f) }
    };

    // ------------------------------------------------------------------
    // 5. River Rocks & Boulders Along the Curved Waterline & Eastern Cliffs
    // ------------------------------------------------------------------
    Rock rockMesh(1.0f, 1);
    std::vector<PropInstance> rockProps = {
        // Rocks in/near Campfire Crescent Cove
        { glm::vec3(-2.8f, -1.06f,  3.2f), glm::vec3(0.85f, 0.50f, 0.85f),  30.0f },
        { glm::vec3(-2.2f, -1.10f,  5.0f), glm::vec3(1.10f, 0.60f, 0.95f),  85.0f },
        { glm::vec3(-1.8f, -1.08f,  2.2f), glm::vec3(0.75f, 0.45f, 0.75f),  55.0f },
        { glm::vec3(-3.2f, -0.90f,  6.0f), glm::vec3(0.95f, 0.52f, 0.90f), 140.0f },

        // Rocks at Southern Spit & Beach
        { glm::vec3( 0.8f, -1.04f, 10.5f), glm::vec3(1.20f, 0.65f, 1.10f), 120.0f },
        { glm::vec3( 1.5f, -1.08f, 13.5f), glm::vec3(1.35f, 0.70f, 1.25f),  45.0f },
        { glm::vec3( 0.5f, -1.05f, 16.5f), glm::vec3(1.05f, 0.55f, 1.00f), 200.0f },
        { glm::vec3(-0.8f, -0.95f, 18.0f), glm::vec3(1.25f, 0.60f, 1.15f),  75.0f },

        // Rocks at Cabin Bluff Promontory
        { glm::vec3( 0.8f, -1.06f, -3.5f), glm::vec3(1.15f, 0.60f, 1.05f),  65.0f },
        { glm::vec3( 1.2f, -1.10f, -6.0f), glm::vec3(1.30f, 0.65f, 1.20f), 150.0f },
        { glm::vec3( 0.5f, -1.04f, -8.5f), glm::vec3(0.90f, 0.50f, 0.80f), 220.0f },

        // Rocks in Northern Shoreline Inlet & Windmill Bay
        { glm::vec3(-0.5f, -1.06f, -16.0f), glm::vec3(1.25f, 0.65f, 1.15f), 310.0f },
        { glm::vec3( 0.2f, -1.08f, -22.0f), glm::vec3(1.40f, 0.70f, 1.30f),  40.0f },
        { glm::vec3( 3.5f, -1.02f, -26.5f), glm::vec3(1.50f, 0.80f, 1.40f),  75.0f },
        { glm::vec3(11.5f, -1.05f, -26.0f), glm::vec3(1.65f, 0.85f, 1.50f), 130.0f },
        { glm::vec3(18.0f, -1.02f, -24.5f), glm::vec3(1.35f, 0.70f, 1.25f), 250.0f },

        // Shoreline rocks along the eastern shore
        { glm::vec3(23.5f, -1.02f, -1.5f), glm::vec3(1.60f, 0.85f, 1.40f),  45.0f },
        { glm::vec3(24.2f, -0.95f, -6.0f), glm::vec3(1.80f, 0.95f, 1.55f), 120.0f },

        // Eastern Southern Shore Rocks
        { glm::vec3(22.5f, -1.04f,  4.5f), glm::vec3(1.30f, 0.65f, 1.20f),  80.0f },
        { glm::vec3(21.8f, -1.08f, 10.0f), glm::vec3(1.45f, 0.75f, 1.30f), 160.0f },
        { glm::vec3(20.5f, -1.05f, 16.0f), glm::vec3(1.20f, 0.60f, 1.10f), 240.0f }
    };

    // Low-poly fallen branches
    Branch branchMesh(1.8f, 0.08f);
    std::vector<PropInstance> branchProps = {
        { glm::vec3(-6.0f, -0.72f,  2.5f), glm::vec3(1.0f),  35.0f },
        { glm::vec3(-9.5f, -0.72f, -4.5f), glm::vec3(1.2f, 1.0f, 1.2f), 115.0f },
        { glm::vec3(-1.2f, -1.02f,  7.5f), glm::vec3(1.1f, 0.9f, 1.1f), 205.0f },
        { glm::vec3(-4.5f, -0.75f,  8.0f), glm::vec3(1.0f, 0.8f, 1.0f),  85.0f }
    };

    // Slender, natural line grass field (thousands of organic line blades across the entire terrain)
    GrassLines grassLines(0.24f, 1.8f);

    // ------------------------------------------------------------------
    // 6. Pine Forest (Campsite Backdrop, North Windmill Ridge & East Cliffs)
    // ------------------------------------------------------------------
    std::vector<PineTree> pineForest = {
        // Behind and surrounding the cabin (Western campsite forest backdrop)
        PineTree(glm::vec3(-14.0f, -0.75f,  -8.0f), 1.55f,  20.0f, 1), // Slender Mountain Pine
        PineTree(glm::vec3(-11.0f, -0.75f, -11.5f), 1.60f, -25.0f, 0), // Compact Dwarf Pine
        PineTree(glm::vec3(-17.5f, -0.75f,  -6.0f), 1.65f,  40.0f, 3), // Wind-Swept Bent Pine
        PineTree(glm::vec3(-19.0f, -0.70f, -12.0f), 1.70f,  10.0f, 2), // Grand Hero Pine
        PineTree(glm::vec3( -9.5f, -0.75f, -14.0f), 1.50f,  30.0f, 4), // Alpine Stunted Pine
        PineTree(glm::vec3(-15.5f, -0.75f, -16.5f), 1.75f, -15.0f, 1),
        PineTree(glm::vec3( -7.0f, -0.75f, -18.0f), 1.45f,  35.0f, 0),

        // Deep western foothills (Creating layered depth in the distance)
        PineTree(glm::vec3(-26.0f,  0.10f, -16.0f), 1.85f,  45.0f, 2),
        PineTree(glm::vec3(-32.0f,  1.10f,  -8.0f), 1.90f, -20.0f, 3),
        PineTree(glm::vec3(-28.0f,  0.40f,   4.0f), 1.75f,  15.0f, 1),
        PineTree(glm::vec3(-35.0f,  1.80f, -22.0f), 2.10f, -30.0f, 2),
        PineTree(glm::vec3(-22.0f, -0.50f,  12.0f), 1.65f,  25.0f, 4),

        // Western foreground / campsite meadow (Framing the meadow edges clear of the cabin and campfire)
        PineTree(glm::vec3(-14.0f, -0.75f,  12.0f), 1.50f, -10.0f, 3), // Wind-Swept Bent Pine (Tree 4 in image)
        PineTree(glm::vec3(-18.0f, -0.65f,  18.0f), 1.65f,  35.0f, 0), // Compact Dwarf Pine (Tree 1 in image)
        PineTree(glm::vec3( -9.0f, -0.75f,  20.0f), 1.40f, -40.0f, 4), // Alpine Stunted Pine (Tree 5 in image)

        // Across the Lake: Framing Northern Shore & Windmill Ridge
        PineTree(glm::vec3( -2.5f, -0.30f, -29.0f), 1.50f,  20.0f, 1),
        PineTree(glm::vec3( -8.0f,  0.10f, -32.0f), 1.60f, -15.0f, 2),
        PineTree(glm::vec3(-14.0f,  0.30f, -28.0f), 1.70f,  40.0f, 3),
        PineTree(glm::vec3( 15.0f, -0.20f, -29.0f), 1.55f, -25.0f, 0),
        PineTree(glm::vec3( 20.0f,  0.15f, -32.0f), 1.65f,  30.0f, 2),
        PineTree(glm::vec3( 25.0f,  0.40f, -28.0f), 1.70f, -10.0f, 1),
        PineTree(glm::vec3(  2.0f,  0.80f, -38.0f), 1.85f,  15.0f, 3),
        PineTree(glm::vec3( 11.0f,  0.90f, -40.0f), 1.90f, -35.0f, 2),
        PineTree(glm::vec3( -5.0f,  1.10f, -42.0f), 2.00f,  25.0f, 4),

        // Across the Lake on the Eastern Shore (Safely framing the hillside clear of the tunnel)
        PineTree(glm::vec3( 56.0f,  0.40f,  -6.0f), 1.80f,  20.0f, 1),
        PineTree(glm::vec3( 36.0f, -0.72f,   6.0f), 1.75f, -15.0f, 2),
        PineTree(glm::vec3( 54.0f,  0.60f, -24.0f), 1.90f,  35.0f, 3),

        // North-East Shore
        PineTree(glm::vec3( 25.0f, -0.75f, -16.0f), 1.50f, -20.0f, 0),
        PineTree(glm::vec3( 30.0f, -0.40f, -20.0f), 1.60f,  40.0f, 4),

        // South-East Shore (Balancing the right foreground)
        PineTree(glm::vec3( 24.0f, -0.75f,   6.0f), 1.45f, -30.0f, 1),
        PineTree(glm::vec3( 26.0f, -0.70f,  12.0f), 1.50f,  45.0f, 2),
        PineTree(glm::vec3( 28.0f, -0.55f,  18.0f), 1.60f, -25.0f, 3),
        PineTree(glm::vec3( 21.0f, -0.75f,  22.0f), 1.40f,  15.0f, 0)
    };

    // ------------------------------------------------------------------
    // 7. Shoreline Campfire & Adirondack Chair (Foreground Left Campsite)
    // ------------------------------------------------------------------
    Campfire campfire(glm::vec3(-3.8f, -0.84f, 4.5f), 1.15f);
    Chair campfireChair(glm::vec3(-5.4f, -0.74f, 5.8f), 36.0f, 1.0f);

    // ------------------------------------------------------------------
    // 8. Main Cabin, Mystical Cave & Lakeside Windmill
    // ------------------------------------------------------------------
    Cube unitCube;
    Pyramid unitPyramid;
    TriangularPrism unitPrism;
    Cylinder unitCylinder;
    Sphere unitCaveSphere(1.0f, 20, 10);
    Sphere unitSphere(1.0f, 32, 20);
    Plane unitCavePlane(1.0f, 1.0f);

    // Cabin nestled in the western meadow, porch facing toward the campfire and lake
    Cabin mainCabin(glm::vec3(-12.5f, -0.72f, -2.0f), 22.0f, 1.0f);

    // Firewood manager handling collectible fallen logs in the meadow outside the cabin
    FirewoodManager firewoodManager;

    // Mystical Cave on the eastern meadow across the lake, opening facing toward campsite
    Cave mysticalCave(glm::vec3(26.0f, -0.72f, -4.0f), -58.0f, 1.15f);

    // Rustic Windmill on the elevated northern meadow bluff across the lake
    Windmill lakesideWindmill(glm::vec3(7.5f, 0.45f, -27.5f), 165.0f, 1.15f);

    // Controllable Wooden Boat moored beside the dock
    Boat lakeBoat(glm::vec3(3.8f, -1.14f, 8.0f), -15.0f, 1.0f);

    // Controllable Hot Air Balloon floating gracefully above the scenic landscape
    HotAirBalloon hotAirBalloon(glm::vec3(12.0f, 4.8f, 2.0f), -45.0f, 1.0f);

    // Beautiful Southern Shore Autumn Landscape Area
    AutumnArea autumnArea;

    // Continuous Falling Rain System (18,000 physical raindrops covering entire full world)
    Rain rain(18000);

    // Atmospheric Sky Thunder System (slight lightning flashes & 3D branching bolts during rain)
    Thunder thunder;

    std::cout << "\n=======================================================\n";
    std::cout << ">>> MoonLake - Interactive 3D Camera Controls Active <<<\n";
    std::cout << "CINEMATIC 2-MINUTE AUTOMATIC FEATURE TOUR:\n";
    std::cout << "  [F1]                     : START / STOP 2-Minute Feature Demonstration Video Tour\n";
    std::cout << "  [SPACE] / [P]            : Pause / Resume Automated Tour Playback\n";
    std::cout << "  [Mouse Drag]             : Full 360-Degree Interactive Camera Look Active during Tour!\n";
    std::cout << "  [Scroll Wheel]           : Dolly / Zoom in & out during Tour\n";
    std::cout << "  [1] - [8]                : Jump directly to Scenes 1 through 8\n";
    std::cout << "CAMERA CONTROLS:\n";
    std::cout << "  [Hold L-CLICK or R-CLICK]: Drag to rotate camera freely in 360 degrees\n";
    std::cout << "  [TAB]                    : Toggle Cursor Lock (FPS Free-Look Mode)\n";
    std::cout << "  [Scroll Wheel]           : Move closer / farther (Dolly in / out)\n";
    std::cout << "  [W] [A] [S] [D]          : Move forward, left, backward, right\n";
    std::cout << "  [LEFT] / [RIGHT] Arrows  : Turn / Rotate view horizontally in 360 degrees (Yaw)\n";
    std::cout << "  [UP] / [DOWN] Arrows     : Tilt / Pitch view up / down (Pitch)\n";
    std::cout << "  [PAGE UP] / [SPACE] / [E]: Move / Fly UP vertically into the sky (Altitude)\n";
    std::cout << "  [PAGE DOWN]/[SHIFT] / [Q]: Move / Fly DOWN vertically towards the ground (Altitude)\n";
    std::cout << "  [L-CTRL]                 : Sprint / Move faster\n";
    std::cout << "  [B]                      : Toggle Controllable Boat Mode (Third-person follow camera + rowboat)\n";
    std::cout << "  [H]                      : Toggle Controllable Hot Air Balloon (Third-Person flight camera)\n";
    std::cout << "  [C]                      : Toggle Guided Cave Tour Mode (Follows path P0 -> P1 -> P2 -> P3)\n";
    std::cout << "  [Y]                      : Teleport directly to Cave Treasure Sanctum (Glowing Gold Coins & Chest)\n";
    std::cout << "  [V]                      : Teleport directly to Lakeside Windmill knoll\n";
    std::cout << "  [K]                      : Teleport directly to Southern Flower Meadow, Train & Station\n";
    std::cout << "  [J]                      : Teleport directly to Cabin Porch (Explore vintage cabin interior)\n";
    std::cout << "  [U]                      : Toggle Controllable Tourist Train (WASD throttle + chase/pilot views)\n";
    std::cout << "  [X]                      : (While on Train) Toggle Camera View (Chase <-> Driver Pilot View)\n";
    std::cout << "  [P]                      : Toggle Tourist Train Motion Pause/Play (Curving railway cruise)\n";
    std::cout << "  [Z]                      : Toggle Train Headlight Spot Light ON/OFF (Dynamic Moving Spot Light)\n";
    std::cout << "  [BACKSPACE] / [0]        : Reset Camera to default campsite overview viewpoint\n";
    std::cout << "CABIN & HEARTH CONTROLS:\n";
    std::cout << "  [O]                      : Open / Close Cabin Front Door (smooth hinged swing)\n";
    std::cout << "  [F] / [E]                : Interact / Pick up firewood / Take torch near stand / Light fireplace\n";
    std::cout << "FLAMING TORCH CONTROLS (Mounted outside cave entrance by default):\n";
    std::cout << "  [T]                      : Take / Mount Flaming Torch (Equip to hand or place back on stand)\n";
    std::cout << "  [G]                      : Cycle Torch Flame Brightness (Dim -> Normal -> Bright -> Blazing)\n";
    std::cout << "WEATHER, LIGHTING & RAY TRACING CONTROLS:\n";
    std::cout << "  [R]                      : Toggle Gold Coins Real-Time Ray Tracing ON/OFF (Multi-bounce radiosity & crevice AO)\n";
    std::cout << "  [4]                      : Toggle Rain & Thunder in Sky ON/OFF (Full scene rainfall & sky lightning)\n";
    std::cout << "  [5]                      : Trigger Thunder Strike in the sky (while raining)\n";
    std::cout << "  [L]                      : Toggle Lighting & Shading ON/OFF (Unlit base colors)\n";
    std::cout << "  [M]                      : Toggle Photorealistic Textures ON/OFF (8 Material Maps)\n";
    std::cout << "  [1] [2] [3]              : Jump directly to Night, Sunset, or Day\n";

    std::cout << "  [ESC]                    : Exit application\n";
    std::cout << "=======================================================\n\n";

    // Environment & celestial state variables (initialized to Night preset)
    EnvironmentPreset currentEnv = NIGHT_PRESET;

    float lastFrameTime = (float)glfwGetTime();

    // ------------------------------------------------------------------
    // 5. Render Loop
    // ------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = (float)glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        if (maxTestFrames > 0)
        {
            if (--maxTestFrames == 0)
            {
                glfwSetWindowShouldClose(window, true);
            }
        }

        // Process WASD and keyboard inputs
        processInput(window, dt, lakeBoat, hotAirBalloon, autumnArea, mainCabin, firewoodManager, mysticalCave, thunder);

        // Update Cabin door animation, fireplace flame flickers, and chimney smoke
        mainCabin.update(dt, currentFrameTime);

        // Update Boat wave harmonics and oar movement
        lakeBoat.update(dt, currentFrameTime);

        // Update Hot Air Balloon atmospheric sway & flame harmonics
        hotAirBalloon.update(dt, currentFrameTime);

        // --------------------------------------------------------------
        // Camera Modes & Cinematic 2-Minute Feature Tour
        // --------------------------------------------------------------
        if (demoTour.isActive())
        {
            demoTour.update(dt, window, camera, lakeBoat, hotAirBalloon, autumnArea,
                            mainCabin, firewoodManager, mysticalCave, thunder,
                            rain, rainEnabled, currentTime, lightingDisabled);
        }
        else if (caveTourActive)
        {
            // Advance along the 3-section path at a smooth pace (~25 seconds total tour)
            caveTourProgress += dt * 0.12f;
            if (caveTourProgress > 3.4f)
            {
                // Tour reached destination (the Grand Sanctum & Treasure Altar)
                caveTourProgress = 3.0f;
            }

            glm::vec3 camPos = mysticalCave.getTourWorldPosition(caveTourProgress);
            glm::vec3 lookTarget = mysticalCave.getTourWorldLookTarget(caveTourProgress);
            camera.setLookAt(camPos, lookTarget);
        }
        else if (boatModeActive)
        {
            // Third-Person Follow Camera aligned smoothly behind the rowboat
            glm::vec3 camPos = lakeBoat.getCameraFollowPosition();
            glm::vec3 lookTarget = lakeBoat.getCameraLookTarget();
            camera.setLookAt(camPos, lookTarget);
        }
        else if (balloonModeActive)
        {
            // Cinematic Third-Person Follow Camera following the Hot Air Balloon
            glm::vec3 camPos = hotAirBalloon.getCameraFollowPosition();
            glm::vec3 lookTarget = hotAirBalloon.getCameraLookTarget();
            camera.setLookAt(camPos, lookTarget);
        }
        else if (trainRideActive)
        {
            // Scenic Tourist Train Ride Camera (Aboard the moving sightseeing train)
            glm::vec3 camPos = autumnArea.getTrainRideCameraPosition();
            glm::vec3 lookTarget = autumnArea.getTrainRideCameraLookTarget();
            camera.setLookAt(camPos, lookTarget);
        }

        // --------------------------------------------------------------
        // Scene Motion Updates (Cleanly decoupled via MotionSystem)
        // --------------------------------------------------------------
        // 1. CPU-side cloud wind drift motion
        MotionSystem::updateClouds(clouds, dt);

        // 1a. Tourist Train Motion along curvy railway loop
        autumnArea.update(dt);

        // 1b. CPU-side continuous translation & recycling motion: Rain (Decoupled in MotionSystem)
        const glm::vec3 rainWind(-2.8f, -22.5f, -1.2f);
        if (rainEnabled)
        {
            MotionSystem::updateRain(rain, dt, rainWind);
            rain.updateBuffers(rainWind);
        }

        // 1c. Atmospheric Sky Thunder motion & lightning envelope
        thunder.update(dt, rainEnabled);

        // 2. Trajectory & sky interpolation motion: Celestial Sun/Moon & Atmosphere
        const EnvironmentPreset* targetPreset = &NIGHT_PRESET;
        if (currentTime == TIME_SUNSET) targetPreset = &SUNSET_PRESET;
        else if (currentTime == TIME_DAY) targetPreset = &DAY_PRESET;

        MotionSystem::updateEnvironmentTransition(currentEnv, *targetPreset, dt);

        glm::vec3 sunDir = glm::normalize(currentEnv.sunPos);
        glm::vec3 moonDir = glm::normalize(currentEnv.moonPos);
        glm::vec3 lightDir = glm::normalize(glm::mix(sunDir, moonDir, currentEnv.moonFactor));

        // Light space matrix for directional shadow map (centered over campsite & lake)
        glm::vec3 shadowCenter(-5.0f, 0.0f, 0.0f);
        glm::mat4 lightProjection = glm::ortho(-70.0f, 70.0f, -70.0f, 70.0f, 1.0f, 220.0f);
        glm::mat4 lightView = glm::lookAt(shadowCenter + lightDir * 85.0f, shadowCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // Precompute motion states needed for both shadow pass and forward pass
        CaveMotesState caveMotes = MotionSystem::computeCaveMotesMotion(currentFrameTime);
        WindmillMotionState windmillMotion = MotionSystem::computeWindmillMotion(currentFrameTime);
        const float fireSpeed = 0.4f;
        FlameMotionState fireMotion = MotionSystem::computeCampfireMotion(currentFrameTime, campfire.getRootMatrix(), fireSpeed);

        // --------------------------------------------------------------
        // PASS 1: Dynamic Directional Shadow Pass (Render depth to FBO)
        // --------------------------------------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);

        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // 1. Mountain Ridges
        shadowShader.setMat4("model", mountainRidgeNorth.getModelMatrix()); mountainRidgeNorth.draw();
        shadowShader.setMat4("model", mountainRidgeNW.getModelMatrix());    mountainRidgeNW.draw();
        shadowShader.setMat4("model", mountainRidgeNE.getModelMatrix());    mountainRidgeNE.draw();
        shadowShader.setMat4("model", mountainRidgeWest.getModelMatrix());  mountainRidgeWest.draw();
        shadowShader.setMat4("model", mountainRidgeEast.getModelMatrix());  mountainRidgeEast.draw();

        // 2. Terrain Ground
        shadowShader.setMat4("model", glm::mat4(1.0f));
        worldTerrain.draw();

        // 3. Pine Trees
        for (const auto& tree : pineForest)
        {
            tree.draw(shadowShader, unitCube, unitPyramid);
        }

        // 4. Props
        mainCabin.draw(shadowShader, unitCube, unitPrism, unitCylinder, unitSphere, unitPyramid);
        campfire.draw(shadowShader, unitCube, unitPyramid, unitCaveSphere, rockMesh, fireMotion);
        campfireChair.draw(shadowShader, unitCube);
        lakesideWindmill.draw(shadowShader, unitCube, unitPyramid, unitCaveSphere, windmillMotion);
        autumnArea.draw(shadowShader, unitCube, unitCylinder, unitCaveSphere, unitPyramid, rockMesh, currentFrameTime);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --------------------------------------------------------------
        // PASS 2: Main Forward Pass (Render scene with lighting & shadows)
        // --------------------------------------------------------------
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Clear screen with current ambient sky tone
        glClearColor(currentEnv.clearColor.r, currentEnv.clearColor.g, currentEnv.clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute Projection & View matrices from active interactive Camera
        float aspectRatio = (height > 0) ? ((float)width / (float)height) : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), aspectRatio, 0.1f, 260.0f);
        glm::mat4 view = camera.getViewMatrix();

        // --------------------------------------------------------------
        // A. Render 360-degree Luminous Atmospheric Sky Dome
        // --------------------------------------------------------------
        glDepthMask(GL_FALSE);
        skyShader.use();
        skyShader.setMat4("projection", projection);
        skyShader.setMat4("view", view);
        glm::mat4 skyModel = glm::translate(glm::mat4(1.0f), glm::vec3(camera.position.x, -5.0f, camera.position.z));
        skyShader.setMat4("model", skyModel);
        glm::vec3 effectiveSkyHorizon = currentEnv.skyHorizon;
        glm::vec3 effectiveSkyZenith  = currentEnv.skyZenith;
        if (thunder.isFlashing())
        {
            float flash = thunder.getFlashIntensity();
            effectiveSkyHorizon += glm::vec3(0.24f, 0.28f, 0.38f) * flash;
            effectiveSkyZenith  += glm::vec3(0.30f, 0.36f, 0.48f) * flash;
        }
        skyShader.setVec3("skyHorizonColor", effectiveSkyHorizon);
        skyShader.setVec3("skyZenithColor", effectiveSkyZenith);
        skyShader.setFloat("horizonY", -5.0f);
        skyShader.setFloat("zenithY", 45.0f);
        skyDome.draw();
        glDepthMask(GL_TRUE);

        // --------------------------------------------------------------
        // B. Render Twinkling Stars (GPU motion driven via MotionSystem)
        // --------------------------------------------------------------
        if (currentEnv.starAlpha > 0.01f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            starsShader.use();
            starsShader.setMat4("projection", projection);
            starsShader.setMat4("view", view);
            MotionSystem::setShaderTime(starsShader, currentFrameTime);
            starsShader.setFloat("starAlpha", currentEnv.starAlpha);
            stars.draw();
            glDisable(GL_BLEND);
        }

        glDepthMask(GL_TRUE); // Re-enable depth writes

        // --------------------------------------------------------------
        // C. Celestial Directional Lighting & Shader Setup
        // --------------------------------------------------------------
        MotionSystem::setShaderTime(mainShader, currentFrameTime);

        mainShader.use();
        mainShader.setMat4("projection", projection);
        mainShader.setMat4("view", view);
        mainShader.setVec3("viewPos", camera.position);
        mainShader.setBool("disableLighting", lightingDisabled);
        mainShader.setFloat("selfIllumination", 0.0f);
        mainShader.setFloat("materialAlpha", 1.0f);
        mainShader.setBool("isGlass", false);
        mainShader.setBool("isCloud", false);
        mainShader.setBool("isGoldCoin", false);
        mainShader.setBool("enableCoinRayTracing", false);
        mainShader.setVec3("moonLightDir", lightDir);

        glm::vec3 effectiveLightColor = currentEnv.lightColor;
        glm::vec3 effectiveAmbientColor = currentEnv.ambientColor;
        if (thunder.isFlashing())
        {
            float flash = thunder.getFlashIntensity();
            effectiveLightColor   += glm::vec3(0.30f, 0.35f, 0.45f) * flash;
            effectiveAmbientColor += glm::vec3(0.12f, 0.15f, 0.22f) * flash;
        }
        mainShader.setVec3("moonLightColor", effectiveLightColor);
        mainShader.setVec3("ambientColor", effectiveAmbientColor);

        // Dynamic Directional Shadow Mapping (Slot 8)
        mainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        mainShader.setBool("shadowsEnabled", !lightingDisabled);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        mainShader.setInt("shadowMap", 8);

        // Bind 2D Realistic Material Texture Maps (GL_TEXTURE0 through GL_TEXTURE7)
        TextureManager::getInstance().bindTextures(mainShader);
        TextureManager::getInstance().setTextureMode(mainShader, TEX_NONE);

        // 3. Dynamic Controllable Flaming Torch (Mounted outside cave entrance by default; carried in hand when taken)
        if (mysticalCave.isTorchLit())
        {
            mainShader.setBool("useCaveLight", true);
            glm::vec3 torchPos = mysticalCave.getActiveTorchLightPos(camera.position, camera.front, camera.right, camera.up);
            mainShader.setVec3("caveLight.position", torchPos);
            float cavePulse = MotionSystem::computeCaveLightPulse(currentFrameTime);
            float torchBright = mysticalCave.getTorchBrightness();
            mainShader.setVec3("caveLight.color", glm::vec3(1.0f, 0.72f, 0.32f) * cavePulse);
            mainShader.setFloat("caveLight.intensity", 1.0f * torchBright);
            mainShader.setFloat("caveLight.constant", 1.0f);
            mainShader.setFloat("caveLight.linear", 0.22f);
            mainShader.setFloat("caveLight.quadratic", 0.12f);
        }
        else
        {
            // Flaming Torch Extinguished - no light inside the cave
            mainShader.setBool("useCaveLight", false);
        }

        // 4. Dynamic Harmonic & Light Motion: Campfire

        glm::vec3 firePos = campfire.getLightPosition();

        mainShader.setBool("useFireLight", true);
        mainShader.setVec3("fireLight.position", firePos);
        mainShader.setVec3("fireLight.color", fireMotion.lightColor);
        mainShader.setFloat("fireLight.intensity", fireMotion.lightIntensity);
        mainShader.setFloat("fireLight.constant", 1.0f);
        mainShader.setFloat("fireLight.linear", 0.10f);
        mainShader.setFloat("fireLight.quadratic", 0.040f);

        // 5. Dynamic Warm Autumn Dock Lantern Light
        glm::vec3 lantPos = autumnArea.getPrimaryLanternLightPos();
        glm::vec3 lantCol = autumnArea.getPrimaryLanternLightColor();
        float lantPulse = 0.94f + 0.06f * std::sin(currentFrameTime * 5.0f);
        mainShader.setBool("useLanternLight", true);
        mainShader.setVec3("lanternLight.position", lantPos);
        mainShader.setVec3("lanternLight.color", lantCol * lantPulse);
        mainShader.setFloat("lanternLight.intensity", 2.2f);
        mainShader.setFloat("lanternLight.constant", 1.0f);
        mainShader.setFloat("lanternLight.linear", 0.11f);
        mainShader.setFloat("lanternLight.quadratic", 0.045f);

        // 6. Dynamic Cabin Fireplace Hearth Point Light (Active when fireplace is lit)
        if (mainCabin.isFireLit())
        {
            mainShader.setBool("useFireplaceLight", true);
            mainShader.setVec3("fireplaceLight.position", mainCabin.getFireplaceLightPosition());
            mainShader.setVec3("fireplaceLight.color", mainCabin.getFireplaceLightColor(currentFrameTime));
            mainShader.setFloat("fireplaceLight.intensity", mainCabin.getFireplaceLightIntensity(currentFrameTime));
            mainShader.setFloat("fireplaceLight.constant", 1.0f);
            mainShader.setFloat("fireplaceLight.linear", 0.12f);
            mainShader.setFloat("fireplaceLight.quadratic", 0.055f);
        }
        else
        {
            mainShader.setBool("useFireplaceLight", false);
        }

        // 7. Localized Cave Treasure Chest & Gold Coins Light (Warm golden glow strictly around the treasure)
        mainShader.setBool("useTreasureLight", true);
        mainShader.setVec3("treasureLight.position", mysticalCave.getTreasureWorldPos());
        mainShader.setVec3("treasureLight.color", mysticalCave.getTreasureLightColor(currentFrameTime));
        mainShader.setFloat("treasureLight.intensity", mysticalCave.getTreasureLightIntensity(currentFrameTime));
        mainShader.setFloat("treasureLight.constant", 1.0f);
        mainShader.setFloat("treasureLight.linear", 0.28f);
        mainShader.setFloat("treasureLight.quadratic", 0.15f);

        // 8. Dynamic Moving Spot Light (Tourist Train Locomotive Headlight Focused Beam)
        if (autumnArea.isTrainHeadlightOn())
        {
            glm::vec3 trainLightPos = autumnArea.getTrainHeadlightPos();
            glm::vec3 trainLightDir = autumnArea.getTrainHeadlightDir();

            mainShader.setBool("useTrainSpotLight", true);
            mainShader.setVec3("trainSpotLight.position", trainLightPos);
            mainShader.setVec3("trainSpotLight.direction", trainLightDir);
            mainShader.setVec3("trainSpotLight.color", glm::vec3(1.0f, 0.94f, 0.76f));
            mainShader.setFloat("trainSpotLight.intensity", 7.0f);
            mainShader.setFloat("trainSpotLight.cutOff", glm::cos(glm::radians(18.0f)));
            mainShader.setFloat("trainSpotLight.outerCutOff", glm::cos(glm::radians(28.0f)));
            mainShader.setFloat("trainSpotLight.constant", 1.0f);
            mainShader.setFloat("trainSpotLight.linear", 0.022f);
            mainShader.setFloat("trainSpotLight.quadratic", 0.0025f);
        }
        else
        {
            mainShader.setBool("useTrainSpotLight", false);
        }

        // 9. Dynamic Moving Point Light (Tourist Train Passenger Coaches Ambient Window Glow)
        mainShader.setBool("useTrainCoachLight", true);
        mainShader.setVec3("trainCoachLight.position", autumnArea.getTrainCoachPosition());
        mainShader.setVec3("trainCoachLight.color", glm::vec3(1.0f, 0.88f, 0.62f));
        mainShader.setFloat("trainCoachLight.intensity", 4.2f);
        mainShader.setFloat("trainCoachLight.constant", 1.0f);
        mainShader.setFloat("trainCoachLight.linear", 0.08f);
        mainShader.setFloat("trainCoachLight.quadratic", 0.025f);

        // --------------------------------------------------------------
        // C2. Render Celestial Bodies (Independent 3D Moon and 3D Sun)
        // --------------------------------------------------------------
        // Disable depth writes so terrestrial mountains and clouds naturally occlude setting celestial bodies
        glDepthMask(GL_FALSE);

        // 1. Render Realistic 3D Moon & Atmospheric Lunar Halo
        if (currentEnv.moonAlpha > 0.01f && currentEnv.moonPos.y > -8.0f)
        {
            glm::mat4 moonHaloBillboard = MotionSystem::computeCelestialBillboard(currentEnv.moonPos, camera.position);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            mainShader.setBool("isMoonGlow", true);
            mainShader.setFloat("celestialAlpha", currentEnv.moonAlpha);
            mainShader.setMat4("model", moonHaloBillboard);
            moonGlow.draw();
            mainShader.setBool("isMoonGlow", false);

            if (currentEnv.moonAlpha < 0.99f)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glDisable(GL_BLEND);
            }
            mainShader.setBool("isMoon", true);
            mainShader.setFloat("celestialAlpha", currentEnv.moonAlpha);
            moon.setPosition(currentEnv.moonPos);
            mainShader.setMat4("model", moon.getModelMatrix());
            moon.draw();
            mainShader.setBool("isMoon", false);
            glDisable(GL_BLEND);
        }

        // 2. Render Realistic 3D Sun & Solar Atmospheric Corona
        if (currentEnv.sunAlpha > 0.01f && currentEnv.sunPos.y > -8.0f)
        {
            glm::mat4 sunCoronaBillboard = MotionSystem::computeCelestialBillboard(currentEnv.sunPos, camera.position);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            mainShader.setBool("isSunGlow", true);
            mainShader.setFloat("sunsetFactor", currentEnv.sunsetFactor);
            mainShader.setFloat("celestialAlpha", currentEnv.sunAlpha);
            mainShader.setMat4("model", sunCoronaBillboard);
            sunGlow.draw();
            mainShader.setBool("isSunGlow", false);

            if (currentEnv.sunAlpha < 0.99f)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glDisable(GL_BLEND);
            }
            mainShader.setBool("isSun", true);
            mainShader.setFloat("sunsetFactor", currentEnv.sunsetFactor);
            mainShader.setFloat("celestialAlpha", currentEnv.sunAlpha);
            sun.setPosition(currentEnv.sunPos);
            mainShader.setMat4("model", sun.getModelMatrix());
            sun.draw();
            mainShader.setBool("isSun", false);
            glDisable(GL_BLEND);
        }

        glDepthMask(GL_TRUE); // Re-enable depth writes for clouds, mountains, terrain, and props

        // --------------------------------------------------------------
        // D. Render 3D Clouds (Hierarchical Modeling)
        // --------------------------------------------------------------
        mainShader.setBool("isCloud", true);
        glm::vec3 effectiveCloudTop = currentEnv.cloudColor;
        glm::vec3 effectiveCloudBottom = currentEnv.cloudBottomColor;
        if (thunder.isFlashing())
        {
            float flash = thunder.getFlashIntensity();
            effectiveCloudTop    += glm::vec3(0.35f, 0.40f, 0.52f) * flash;
            effectiveCloudBottom += glm::vec3(0.25f, 0.30f, 0.42f) * flash;
        }
        mainShader.setVec3("cloudTopColor", effectiveCloudTop);
        mainShader.setVec3("cloudBottomColor", effectiveCloudBottom);
        for (const auto& cloud : clouds)
        {
            cloud.draw(mainShader, cloudPuffSphere);
        }
        mainShader.setBool("isCloud", false);


        // --------------------------------------------------------------
        // E. Render Distant Snow-Capped Alpine Mountains (Valley Amphitheater)
        // --------------------------------------------------------------
        TextureManager::getInstance().setTextureMode(mainShader, TEX_ROCK, 0.10f);
        mainShader.setFloat("shininess", 8.0f);
        mainShader.setMat4("model", mountainRidgeNorth.getModelMatrix());
        mountainRidgeNorth.draw();

        mainShader.setMat4("model", mountainRidgeNW.getModelMatrix());
        mountainRidgeNW.draw();

        mainShader.setMat4("model", mountainRidgeNE.getModelMatrix());
        mountainRidgeNE.draw();

        mainShader.setMat4("model", mountainRidgeWest.getModelMatrix());
        mountainRidgeWest.draw();

        mainShader.setMat4("model", mountainRidgeEast.getModelMatrix());
        mountainRidgeEast.draw();

        // --------------------------------------------------------------
        // F. Render Terrain: Continuous Organic Mesh & Cave Cliff Shelves
        // --------------------------------------------------------------
        TextureManager::getInstance().setTextureMode(mainShader, TEX_GRASS, 0.35f);
        mainShader.setFloat("shininess", 16.0f);

        // 1. Continuous Organic 3D Terrain Mesh across the Entire World
        // With objectColor = vec3(0.0f), the shader automatically uses smooth per-vertex colors!
        mainShader.setMat4("model", glm::mat4(1.0f));
        mainShader.setVec3("objectColor", glm::vec3(0.0f));
        worldTerrain.draw();

        // --------------------------------------------------------------
        // G. Render Props: Dock, Boat, Rocks, Branches, Grass, Cabin, Cave
        // --------------------------------------------------------------
        // 1. Wooden Dock
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 1.2f);
        mainShader.setFloat("shininess", 12.0f);
        for (const auto& d : dockPieces)
        {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), d.pos);
            if (d.rotY != 0.0f) m = glm::rotate(m, glm::radians(d.rotY), glm::vec3(0, 1, 0));
            m = glm::scale(m, d.scale);
            mainShader.setMat4("model", m);
            mainShader.setVec3("objectColor", d.color);
            unitCube.draw();
        }

        // 1b. Controllable Wooden Rowboat (Hierarchical model with bobbing & oars)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 1.0f);
        lakeBoat.draw(mainShader, unitCube, unitPrism, unitCylinder);

        // 1c. Controllable Hot Air Balloon (Multi-segment envelope, burner flame, wicker basket)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_NONE);
        hotAirBalloon.draw(mainShader, unitCube, unitCylinder, unitSphere, unitPyramid);

        // 2. Shoreline River Rocks & Cliff Boulders (Curved Placement)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_ROCK, 0.55f);
        mainShader.setFloat("shininess", 32.0f);
        for (const auto& r : rockProps)
        {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), r.pos);
            m = glm::rotate(m, glm::radians(r.rotY), glm::vec3(0, 1, 0));
            m = glm::scale(m, r.scale);
            mainShader.setMat4("model", m);
            rockMesh.draw();
        }

        // 4. Slender Natural Line Grass Field (Thousands of organic line blades across the ground)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_NONE);
        mainShader.setFloat("shininess", 6.0f);
        mainShader.setMat4("model", glm::mat4(1.0f));
        mainShader.setVec3("objectColor", glm::vec3(0.0f)); // Use blade vertex color gradients
        grassLines.draw();

        // 5. Main Wooden Camping Cabin (With openable door, vintage interior, fireplace & chimney)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 0.85f);
        mainCabin.draw(mainShader, unitCube, unitPrism, unitCylinder, unitSphere, unitPyramid);

        // 5b. Collectible Fallen Branches & Wood Logs scattered on the ground around the cabin & meadow
        TextureManager::getInstance().setTextureMode(mainShader, TEX_BARK, 0.8f);
        firewoodManager.draw(mainShader, unitCylinder, unitCube, unitSphere, branchMesh, currentFrameTime);

        // 6. Tall Evergreen Pine Forest (Stylized Pines Matching Reference Image)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_NONE, 1.0f);
        for (const auto& tree : pineForest)
        {
            tree.draw(mainShader, unitCube, unitPyramid);
        }

        // 7. Shoreline Campfire & Chair (On the Left Beach Cove)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_ROCK, 0.5f);
        campfire.draw(mainShader, unitCube, unitPyramid, unitCaveSphere, rockMesh, fireMotion);
        campfireChair.draw(mainShader, unitCube);

        // 8. Mystical Cave (Cobblestone Tunnel - Naturally dark inside with NO light source on the wall)
        // Outside directional sky light (moon/sun) does not penetrate the enclosed stone vault
        mainShader.setVec3("moonLightColor", glm::vec3(0.0f));
        mainShader.setVec3("ambientColor", glm::vec3(0.035f, 0.035f, 0.040f));
        TextureManager::getInstance().setTextureMode(mainShader, TEX_COBBLESTONE, 0.5f);

        mysticalCave.draw(mainShader, unitCube, unitPrism, unitPyramid, unitCaveSphere, unitCavePlane, unitCylinder, rockMesh, caveMotes, currentFrameTime);

        // Restore sky directional and ambient lighting for subsequent scene props
        mainShader.setVec3("moonLightColor", currentEnv.lightColor);
        mainShader.setVec3("ambientColor", currentEnv.ambientColor);

        // 8b. Render Player's Handheld Flaming Torch in first-person (User controls this torch)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 1.0f);
        mysticalCave.drawHandheldTorch(mainShader, unitCube, unitCylinder, unitCaveSphere, unitPyramid, camera.position, camera.front, camera.right, camera.up, currentFrameTime);

        // 9. Lakeside Windmill (Across the lake on the far northern shore)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 0.75f);
        lakesideWindmill.draw(mainShader, unitCube, unitPyramid, unitCaveSphere, windmillMotion);

        // 10. Beautiful Autumn Landscape (Across the lake on the southern shore)
        TextureManager::getInstance().setTextureMode(mainShader, TEX_WOOD, 0.8f);
        autumnArea.draw(mainShader, unitCube, unitCylinder, unitCaveSphere, unitPyramid, rockMesh, currentFrameTime);

        // Reset texture mode to none for subsequent non-textured meshes
        TextureManager::getInstance().setTextureMode(mainShader, TEX_NONE);

        // --------------------------------------------------------------
        // H. Render Animated Reflective Lake Water (GPU motion via MotionSystem)
        // --------------------------------------------------------------
        waterShader.use();
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view", view);
        waterShader.setMat4("model", lake.getModelMatrix());
        MotionSystem::setShaderTime(waterShader, currentFrameTime);
        waterShader.setVec3("lakeCenter", glm::vec3(12.0f, -1.18f, -4.0f));
        waterShader.setVec2("lakeRadius", glm::vec2(18.0f, 28.0f));
        waterShader.setVec3("viewPos", camera.position);
        waterShader.setVec3("moonLightDir", lightDir);
        glm::vec3 waterLightColor = currentEnv.lightColor;
        glm::vec3 waterAmbientColor = currentEnv.ambientColor;
        glm::vec3 waterSkyHorizon = currentEnv.skyHorizon;
        if (thunder.isFlashing())
        {
            float flash = thunder.getFlashIntensity();
            waterLightColor   += glm::vec3(0.30f, 0.35f, 0.45f) * flash;
            waterAmbientColor += glm::vec3(0.12f, 0.15f, 0.22f) * flash;
            waterSkyHorizon   += glm::vec3(0.24f, 0.28f, 0.38f) * flash;
        }
        waterShader.setVec3("moonLightColor", waterLightColor);
        waterShader.setVec3("ambientColor", waterAmbientColor);
        waterShader.setVec3("skyHorizonColor", waterSkyHorizon);
        waterShader.setVec3("waterBaseColor", currentEnv.waterBaseColor);

        // Dynamic Campfire Water Specular Reflection (Matching Image 1)
        waterShader.setBool("hasCampfire", true);
        waterShader.setVec3("campfirePos", firePos);
        waterShader.setVec3("campfireColor", fireMotion.lightColor);
        waterShader.setFloat("campfireIntensity", fireMotion.lightIntensity);

        // Dynamic Autumn Dock Lantern Water Specular Reflection
        waterShader.setBool("hasLantern", true);
        waterShader.setVec3("lanternPos", lantPos);
        waterShader.setVec3("lanternColor", lantCol);
        waterShader.setFloat("lanternIntensity", 2.2f * lantPulse);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        lake.draw();
        glDisable(GL_BLEND);

        // --------------------------------------------------------------
        // I. Render Continuous Falling Rain (Slanted translucent streaks)
        // --------------------------------------------------------------
        if (rainEnabled)
        {
            rainShader.use();
            rainShader.setMat4("projection", projection);
            rainShader.setMat4("view", view);
            rainShader.setMat4("model", glm::mat4(1.0f));
            float rainAlpha = (currentTime == TIME_NIGHT) ? 0.72f : (currentTime == TIME_SUNSET ? 0.55f : 0.40f);
            if (thunder.isFlashing()) rainAlpha = std::min(1.0f, rainAlpha + thunder.getFlashIntensity() * 0.15f);
            rainShader.setFloat("rainAlpha", rainAlpha);
            rain.draw();
        }

        // --------------------------------------------------------------
        // J. Render Visible 3D Branching Lightning Bolt in the Sky
        // --------------------------------------------------------------
        if (thunder.isFlashing())
        {
            rainShader.use();
            rainShader.setMat4("projection", projection);
            rainShader.setMat4("view", view);
            rainShader.setMat4("model", glm::mat4(1.0f));
            rainShader.setFloat("rainAlpha", thunder.getFlashIntensity());
            thunder.draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Rotate camera if tour is active (allowing simultaneous 360 look)
    // or if manual Left/Right mouse is dragging OR cursor is locked
    if (demoTour.isActive())
    {
        demoTour.processMouseMovement(xoffset, yoffset, isDragging || cursorLocked);
    }
    else if (isDragging || cursorLocked)
    {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            isDragging = true;
            firstMouse = true; // Reset to avoid jump when starting drag
        }
        else if (action == GLFW_RELEASE)
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS &&
                glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
            {
                isDragging = false;
            }
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (demoTour.isActive())
    {
        demoTour.processMouseScroll(static_cast<float>(yoffset));
    }
    else
    {
        camera.processMouseScroll(static_cast<float>(yoffset));
    }
}

// Input processing with key debouncing for toggles
bool tKeyPressed = false;
bool gKeyPressed = false;
bool tabKeyPressed = false;
bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool rKeyPressed = false;
bool lKeyPressed = false;
bool yKeyPressed = false;

void processInput(GLFWwindow* window, float dt, Boat& boat, HotAirBalloon& balloon, AutumnArea& autumnArea, Cabin& cabin, FirewoodManager& firewood, Cave& cave, Thunder& thunder)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle 2-Minute Automatic Cinematic Feature Showcase Tour with [F1]
    static bool f1KeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        if (!f1KeyPressed)
        {
            f1KeyPressed = true;
            demoTour.toggleActive();
            if (demoTour.isActive())
            {
                caveTourActive = false;
                boatModeActive = false;
                balloonModeActive = false;
                trainRideActive = false;
                camera.clampYaw = false;
            }
        }
    }
    else
    {
        f1KeyPressed = false;
    }

    // When the 2-Minute Cinematic Tour is active, handle tour controls
    if (demoTour.isActive())
    {
        // [SPACE] or [P]: Pause / Resume Tour Timeline Playback
        static bool pauseKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            if (!pauseKeyPressed)
            {
                pauseKeyPressed = true;
                demoTour.togglePause();
            }
        }
        else
        {
            pauseKeyPressed = false;
        }

        // Keys [1] through [8]: Jump directly to any of the 8 feature scenes
        static int activeJumpKey = -1;
        bool anyNumberPressed = false;
        for (int s = 0; s < 8; ++s)
        {
            int key = GLFW_KEY_1 + s;
            int kpKey = GLFW_KEY_KP_1 + s;
            if (glfwGetKey(window, key) == GLFW_PRESS || glfwGetKey(window, kpKey) == GLFW_PRESS)
            {
                anyNumberPressed = true;
                if (activeJumpKey != s)
                {
                    activeJumpKey = s;
                    demoTour.jumpToScene(s);
                }
                break;
            }
        }
        if (!anyNumberPressed)
        {
            activeJumpKey = -1;
        }

        // The automated spline flight and feature triggers handle motion.
        // Full 360-degree interactive camera look is actively handled via mouse_callback.
        return;
    }

    // Toggle Boat Control & Third-Person Camera Mode with [B]
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        if (!bKeyPressed)
        {
            bKeyPressed = true;
            boatModeActive = !boatModeActive;
            if (boatModeActive)
            {
                caveTourActive = false;
                balloonModeActive = false;
                trainRideActive = false;
                camera.clampYaw = false;
                std::cout << "\n=======================================================\n";
                std::cout << ">>> CONTROLLABLE BOAT MODE ENGAGED <<<\n";
                std::cout << "  Camera: Smooth Third-Person Following View\n";
                std::cout << "  [W]        : Row forward (accelerate)\n";
                std::cout << "  [S]        : Reverse / backwater\n";
                std::cout << "  [A] / [D]  : Steer port (left) / starboard (right)\n";
                std::cout << "  Bobbing: y(t) = y0 + A * sin(omega * t)\n";
                std::cout << "  (Press [B] again to return to Free-Look Camera)\n";
                std::cout << "=======================================================\n\n";
            }
            else
            {
                std::cout << "[Boat Mode] Disengaged. Returned to Free-Look Camera.\n";
            }
        }
    }
    else
    {
        bKeyPressed = false;
    }

    // Toggle Hot Air Balloon Control & Flight Camera with [H]
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        if (!hKeyPressed)
        {
            hKeyPressed = true;
            balloonModeActive = !balloonModeActive;
            if (balloonModeActive)
            {
                caveTourActive = false;
                boatModeActive = false;
                trainRideActive = false;
                camera.clampYaw = false;
                std::cout << "\n=======================================================\n";
                std::cout << ">>> CONTROLLABLE HOT AIR BALLOON ENGAGED <<<\n";
                std::cout << "  Camera: Cinematic Flight Following View\n";
                std::cout << "  [W] / [S]        : Fly Forward / Backward\n";
                std::cout << "  [A] / [D]        : Yaw / Steer Left / Right\n";
                std::cout << "  [SPACE] / [E]    : Fire Burner -> Ascend into the sky\n";
                std::cout << "  [L-SHIFT] / [Q]  : Vent hot air -> Descend\n";
                std::cout << "  [L-CTRL]         : Burner Boost / Accelerated flight\n";
                std::cout << "  (Press [H] again to return to Free-Look Camera)\n";
                std::cout << "=======================================================\n\n";
            }
            else
            {
                std::cout << "[Hot Air Balloon] Disengaged. Returned to Free-Look Camera.\n";
            }
        }
    }
    else
    {
        hKeyPressed = false;
    }

    // Toggle Controllable Tourist Train Ride Mode with [U]
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        if (!uKeyPressed)
        {
            uKeyPressed = true;
            trainRideActive = !trainRideActive;
            if (trainRideActive)
            {
                caveTourActive = false;
                boatModeActive = false;
                balloonModeActive = false;
                camera.clampYaw = false;
                std::cout << "\n=======================================================\n";
                std::cout << ">>> CONTROLLABLE TOURIST TRAIN ENGAGED <<<\n";
                std::cout << "  Camera: Scenic Third-Person Chase & Driver Pilot View\n";
                std::cout << "  [W]        : Throttle Up / Accelerate forward along tracks\n";
                std::cout << "  [S]        : Decelerate / Apply brakes / Reverse gear\n";
                std::cout << "  [SPACE]    : Emergency brake (full stop)\n";
                std::cout << "  [P]        : Toggle Pause / Resume cruising\n";
                std::cout << "  [X]        : Toggle Camera View (Chase View <-> Driver Pilot View)\n";
                std::cout << "  (Press [U] again to return to Free-Look Camera)\n";
                std::cout << "=======================================================\n\n";
            }
            else
            {
                std::cout << "[Tourist Train] Disengaged. Returned to Free-Look Camera.\n";
            }
        }
    }
    else
    {
        uKeyPressed = false;
    }

    // Toggle Camera View Mode with [X] (while on train)
    if (trainRideActive && glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        if (!xKeyPressed)
        {
            xKeyPressed = true;
            autumnArea.toggleTrainCameraView();
            std::cout << "[Train Camera] Switched to: "
                      << (autumnArea.getTrainCameraViewMode() == 1 ? "Driver's Front Pilot View" : "Scenic Third-Person Chase View")
                      << std::endl;
        }
    }
    else
    {
        xKeyPressed = false;
    }

    // Toggle Tourist Train Motion Pause/Play with [P]
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!pKeyPressed)
        {
            pKeyPressed = true;
            autumnArea.toggleTrainMoving();
            std::cout << "[Train Motion] "
                      << (autumnArea.isTrainMoving() ? "RUNNING (Cruising along curvy railway)" : "PAUSED (Stopped at current track position)")
                      << std::endl;
        }
    }
    else
    {
        pKeyPressed = false;
    }

    // Toggle Tourist Train Headlight Spot Light with [Z]
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        if (!zKeyPressed)
        {
            zKeyPressed = true;
            autumnArea.toggleTrainHeadlight();
            std::cout << "[Spot Light] Train Locomotive Headlight beam turned "
                      << (autumnArea.isTrainHeadlightOn() ? "ON (Focused Beam Active)" : "OFF")
                      << std::endl;
        }
    }
    else
    {
        zKeyPressed = false;
    }

    // Toggle Photorealistic Textures ON/OFF with [M]
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!mKeyPressed)
        {
            mKeyPressed = true;
            TextureManager::getInstance().toggleEnabled();
            std::cout << "[Textures] Photorealistic Material Texturing "
                      << (TextureManager::getInstance().isEnabled() ? "ENABLED (7 Material Texture Maps Active)" : "DISABLED (Base Colors Only)")
                      << std::endl;
        }
    }
    else
    {
        mKeyPressed = false;
    }

    // Sprint modifier: Hold Left-Ctrl or Right-Shift to move 2.5x faster
    bool fast = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                 glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // If Boat Mode is active, WASD steers and drives the boat
    if (boatModeActive)
    {
        bool forward  = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
        bool backward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
        bool left     = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        bool right    = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);

        boat.processInput(forward, backward, left, right, dt);
    }
    else if (balloonModeActive)
    {
        bool forward   = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
        bool backward  = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
        bool turnLeft  = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        bool turnRight = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
        bool ascend    = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
        bool descend   = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
        bool boost     = fast;

        balloon.processInput(forward, backward, turnLeft, turnRight, ascend, descend, boost, dt);
    }
    else if (trainRideActive)
    {
        bool accelerate     = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
        bool brake          = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
        bool emergencyBrake = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        autumnArea.processTrainInput(accelerate, brake, emergencyBrake, dt);
    }
    else
    {
        // If manual movement is pressed while in Cave Tour mode, disengage to give user full control
        if (caveTourActive && (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                               glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
        {
            caveTourActive = false;
            std::cout << "[Cave Tour] Manual movement detected - resumed Free-Look Camera.\n";
        }

        // 1. Move horizontally: W (forward), S (backward), A (strafe left), D (strafe right)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(CAM_FORWARD, dt, fast);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(CAM_BACKWARD, dt, fast);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(CAM_LEFT, dt, fast);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(CAM_RIGHT, dt, fast);

        // 2. Full 360-degree camera rotation with Keyboard Arrow Keys (Left, Right, Up, Down)
        float yawRot = 0.0f;
        float pitchRot = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
            yawRot -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
            yawRot += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
            pitchRot += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
            pitchRot -= 1.0f;

        if (yawRot != 0.0f || pitchRot != 0.0f)
            camera.processKeyboardRotation(yawRot, pitchRot, dt);
    }

    // 3. Vertical Flight / Elevation (Move UP / DOWN)
    // [PAGE UP], [SPACE], or [E] -> Move camera UP vertically into the sky
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.processKeyboard(CAM_UP, dt, fast);
    }

    // [PAGE DOWN], [LEFT SHIFT], or [Q] -> Move camera DOWN vertically towards the ground
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.processKeyboard(CAM_DOWN, dt, fast);
    }

    // Weather: Toggle Continuous Rain with [4]
    static bool key4Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
    {
        if (!key4Pressed)
        {
            key4Pressed = true;
            rainEnabled = !rainEnabled;
            std::cout << "[Weather] Continuous Rain toggled "
                      << (rainEnabled ? "ON (Full Scene Coverage + Sky Thunder Active)" : "OFF (Sky Clear)")
                      << std::endl;
        }
    }
    else
    {
        key4Pressed = false;
    }

    // Weather: Trigger Thunder Strike with [5]
    static bool key5Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
    {
        if (!key5Pressed)
        {
            key5Pressed = true;
            if (!rainEnabled)
            {
                rainEnabled = true;
                std::cout << "[Weather] Continuous Rain toggled ON for thunder strike!\n";
            }
            thunder.triggerStrike();
        }
    }
    else
    {
        key5Pressed = false;
    }

    // Real-Time Ray Tracing: Toggle Gold Coins Ray Tracing ON/OFF with [R]
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (!rKeyPressed)
        {
            rKeyPressed = true;
            cave.toggleCoinRayTracing();
            std::cout << "\n=======================================================\n";
            if (cave.isCoinRayTracingEnabled())
            {
                std::cout << ">>> [RAY TRACING ON] Gold Coins Real-Time Ray Tracing: ACTIVE <<<\n";
                std::cout << "  - Module: shaders/raytracing_coins.glsl (Standalone & modular)\n";
                std::cout << "  - Secondary Reflection Rays: Multi-bounce metallic radiosity\n";
                std::cout << "    (Gold-on-gold amber glow, glowing orb reflections, mahogany chest walls)\n";
                std::cout << "  - Secondary Shadow Rays: Tracing contact crevice ambient occlusion\n";
                std::cout << "  - Conductor Fresnel: F0 = (1.0, 0.78, 0.34) gold metal reflectivity\n";
            }
            else
            {
                std::cout << ">>> [RAY TRACING OFF] Gold Coins Ray Tracing: DISABLED <<<\n";
                std::cout << "  - Standard rasterized Blinn-Phong shading active for comparison.\n";
            }
            std::cout << "=======================================================\n\n";
        }
    }
    else
    {
        rKeyPressed = false;
    }


    // Lighting & Shading: Toggle on/off with [L] to see pure unlit base colors
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        if (!lKeyPressed)
        {
            lKeyPressed = true;
            lightingDisabled = !lightingDisabled;
            std::cout << "[Shading] Lighting & Shading "
                      << (lightingDisabled ? "DISABLED (Displaying unlit pure base colors)" : "ENABLED (Full Blinn-Phong shading)")
                      << std::endl;
        }
    }
    else
    {
        lKeyPressed = false;
    }

    // Reset Camera with [BACKSPACE] or [0] (Overview vantage point)
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        caveTourActive = false;
        boatModeActive = false;
        balloonModeActive = false;
        trainRideActive = false;
        camera.position = glm::vec3(-1.2f, 2.9f, 13.8f);
        camera.yaw = -87.0f;
        camera.pitch = -7.0f;
        camera.zoom = 45.0f;
    }

    // Toggle Guided Cave Tour Mode with [C] (Navigates automatically through P0 -> P1 -> P2 -> P3)
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        if (!cKeyPressed)
        {
            cKeyPressed = true;
            caveTourActive = !caveTourActive;
            if (caveTourActive)
            {
                boatModeActive = false;
                balloonModeActive = false;
                trainRideActive = false;
                caveTourProgress = 0.0f;
                camera.clampYaw = false;
                cave.setPlayerCarryingTorch(true); // Automatically equip torch for the dark cave tour
                std::cout << "\n=======================================================\n";
                std::cout << ">>> GUIDED CAVE TOUR MODE STARTED <<<\n";
                std::cout << "  Camera automatically following mathematical path:\n";
                std::cout << "  P0 (Entrance) -> P1 (Vestibule) -> P2 (Cavern) -> P3 (Treasure Sanctum)\n";
                std::cout << "  Interpolation: C(t) = (1 - t)*P_i + t*P_{i+1}\n";
                std::cout << "  (Press [C] again or WASD to disengage tour)\n";
                std::cout << "=======================================================\n\n";
            }
            else
            {
                std::cout << "[Cave Tour] Tour STOPPED. Resumed Free Camera.\n";
            }
        }
    }
    else
    {
        cKeyPressed = false;
    }

    // Teleport directly to Cave Treasure Sanctum (Glowing Gold Coins & Chest) with [Y]
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        if (!yKeyPressed)
        {
            yKeyPressed = true;
            caveTourActive = false;
            boatModeActive = false;
            balloonModeActive = false;
            trainRideActive = false;
            cave.setPlayerCarryingTorch(true); // Automatically equip torch for inner cavern
            glm::vec3 treasureCam = cave.getTreasureViewCameraPos();
            glm::vec3 treasureLook = cave.getTreasureViewLookTarget();
            camera.setLookAt(treasureCam, treasureLook);
            camera.clampYaw = false;
            std::cout << "\n=======================================================\n";
            std::cout << ">>> [TELEPORT] Cave Treasure Sanctum <<<\n";
            std::cout << "  Arrived at the glowing gold coins hoard and open treasure chest!\n";
            std::cout << "  Controls: [R] Toggle Ray Tracing | [T] Place/Take Torch | [G] Torch Brightness\n";
            std::cout << "=======================================================\n\n";
        }
    }
    else
    {
        yKeyPressed = false;
    }

    // Teleport directly to Lakeside Windmill Knoll with [V]
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        camera.position = glm::vec3(7.5f, 1.8f, -21.0f);
        camera.yaw = -90.0f;
        camera.pitch = -6.0f;
    }

    // Teleport directly to Southern Flower Meadow, Train & Station with [K]
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        camera.position = glm::vec3(10.5f, 1.6f, 17.5f);
        camera.yaw = 90.0f;
        camera.pitch = 8.0f;
    }

    // Teleport directly to Cabin Porch with [J]
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        if (!jKeyPressed)
        {
            jKeyPressed = true;
            caveTourActive = false;
            boatModeActive = false;
            balloonModeActive = false;
            trainRideActive = false;
            camera.position = glm::vec3(-11.5f, 0.40f, 3.5f);
            camera.yaw = -158.0f;
            camera.pitch = -4.0f;
            camera.clampYaw = false;
            std::cout << "[Teleport] Arrived at Cabin Porch. Press [O] to open door, [F] to pick wood or lit fire!\n";
        }
    }
    else
    {
        jKeyPressed = false;
    }

    // Toggle Cabin Door directly with [O]
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        if (!oKeyPressed)
        {
            oKeyPressed = true;
            cabin.toggleDoor();
            std::cout << "[Cabin Door] Door " << (cabin.isDoorOpen() ? "OPENED." : "CLOSED.") << std::endl;
        }
    }
    else
    {
        oKeyPressed = false;
    }

    // Proximity Interaction with [F] or [E]: Pick up fallen wood, open door, light fireplace
    bool interactTriggered = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fKeyPressed)
        {
            fKeyPressed = true;
            interactTriggered = true;
        }
    }
    else
    {
        fKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !balloonModeActive)
    {
        if (!eKeyPressed)
        {
            eKeyPressed = true;
            interactTriggered = true;
        }
    }
    else
    {
        eKeyPressed = false;
    }

    if (interactTriggered)
    {
        // 1. Check if near uncollected fallen branch or wood log outside
        int woodIdx = -1;
        if (firewood.tryCollectNear(camera.position, 5.0f, &woodIdx))
        {
            std::cout << "\n=======================================================\n";
            std::cout << ">>> [Firewood Gathered!] Picked up fallen wood item #" << (woodIdx + 1) << "! <<<\n";
            std::cout << "    Inventory: " << firewood.getCollectedCount() << " / " << firewood.getTotalCount() << " wood items gathered.\n";
            std::cout << "    Walk inside the cabin to the arched fireplace and press [F] or [E] to lit it!\n";
            std::cout << "=======================================================\n\n";
        }
        else
        {
            // 2. Check if near fireplace inside cabin (Cylindrical horizontal check)
            glm::vec3 fpPos = cabin.getFireplaceWorldPosition();
            float dxFp = camera.position.x - fpPos.x;
            float dzFp = camera.position.z - fpPos.z;
            float horizDistFp = std::sqrt(dxFp * dxFp + dzFp * dzFp);
            float vertDistFp = std::abs(camera.position.y - fpPos.y);

            if (horizDistFp <= 4.5f && vertDistFp <= 3.5f)
            {
                if (firewood.getCollectedCount() > 0)
                {
                    if (!cabin.isFireLit())
                    {
                        cabin.setFireLit(true);
                        cabin.addWood();
                        std::cout << "\n=======================================================\n";
                        std::cout << ">>> CABIN FIREPLACE IGNITED! <<<\n";
                        std::cout << "  Placed gathered firewood into the arched brick hearth!\n";
                        std::cout << "  Crackling roaring fire warms the cabin interior!\n";
                        std::cout << "  Chimney is now releasing billows of smoke into the sky!\n";
                        std::cout << "=======================================================\n\n";
                    }
                    else
                    {
                        cabin.addWood();
                        std::cout << "[Fireplace] Added more wood to the hearth! Total logs fed: " << cabin.getWoodCount() << ".\n";
                    }
                }
                else
                {
                    std::cout << "[Fireplace] The hearth is cold and empty! Explore the meadow outside and press [F] near glowing fallen branches or wood logs to collect firewood first!\n";
                }
            }
            else
            {
                // 3. Check if near cabin door
                glm::vec3 doorPos = cabin.getDoorWorldPosition();
                float dxDoor = camera.position.x - doorPos.x;
                float dzDoor = camera.position.z - doorPos.z;
                float horizDistDoor = std::sqrt(dxDoor * dxDoor + dzDoor * dzDoor);
                float vertDistDoor = std::abs(camera.position.y - doorPos.y);

                if (horizDistDoor <= 4.0f && vertDistDoor <= 3.5f)
                {
                    cabin.toggleDoor();
                    std::cout << "[Cabin Door] Door " << (cabin.isDoorOpen() ? "OPENED." : "CLOSED.") << std::endl;
                }
                else
                {
                    // 4. Check if near torch stand outside cave entrance
                    glm::vec3 torchStandPos = cave.getTorchStandWorldPos();
                    float distTorch = glm::distance(camera.position, torchStandPos);
                    if (distTorch <= 3.8f)
                    {
                        cave.togglePlayerTorch();
                        std::cout << "\n=======================================================\n";
                        if (cave.isPlayerCarryingTorch())
                        {
                            std::cout << ">>> [FLAMING TORCH TAKEN] Picked up the Flaming Torch into your hand! <<<\n";
                            std::cout << "  Torch is now carried in first-person with reduced, cozy illumination radius.\n";
                            std::cout << "  Press [T] (or [F] near stand) to place it back on the entrance stand, or [G] to adjust brightness.\n";
                        }
                        else
                        {
                            std::cout << ">>> [FLAMING TORCH PLACED] Returned the Flaming Torch to the entrance stand! <<<\n";
                            std::cout << "  Torch is now mounted outside the cave entrance.\n";
                            std::cout << "  Press [T] (or [F]/[E] near stand) anytime to take it into your hand!\n";
                        }
                        std::cout << "=======================================================\n\n";
                    }
                    else
                    {
                        // 5. Prompt hint for user
                        float nearestDist = 999.0f;
                        glm::vec3 nearestPos;
                        if (firewood.getNearestUncollected(camera.position, &nearestDist, &nearestPos))
                        {
                            std::cout << "[Interaction] No firewood in pickup range (Nearest wood is ~"
                                      << int(nearestDist + 0.5f)
                                      << "m away). Walk closer to the glowing fallen branches/logs to collect them with [F]!\n";
                        }
                    }
                }
            }
        }
    }

    // Toggle Cursor Lock with [TAB]
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!tabKeyPressed)
        {
            tabKeyPressed = true;
            cursorLocked = !cursorLocked;
            glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            if (cursorLocked) firstMouse = true;
            std::cout << "[Camera] Mouse Cursor " << (cursorLocked ? "LOCKED (FPS Free-Look Mode)" : "UNLOCKED (Click & Drag to rotate)") << std::endl;
        }
    }
    else
    {
        tabKeyPressed = false;
    }

    // Flaming Torch Controls: Toggle Take / Place Torch with [T]
    // "keep it outside the cave entrance, i can take it when i want."
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (!tKeyPressed)
        {
            tKeyPressed = true;
            cave.togglePlayerTorch();
            std::cout << "\n=======================================================\n";
            if (cave.isPlayerCarryingTorch())
            {
                std::cout << ">>> [FLAMING TORCH TAKEN] You picked up the Flaming Torch into your hand! <<<\n";
                std::cout << "  Torch is now carried in first-person with reduced, cozy illumination radius.\n";
                std::cout << "  Full torch is visible on screen. Press [T] to place it back on the stand outside, or [G] to adjust brightness.\n";
            }
            else
            {
                std::cout << ">>> [FLAMING TORCH PLACED] Placed the Flaming Torch back on the entrance stand! <<<\n";
                std::cout << "  Torch is standing outside the cave entrance.\n";
                std::cout << "  Press [T] (or [F]/[E] near stand) anytime to take it into your hand!\n";
            }
            std::cout << "=======================================================\n\n";
        }
    }
    else
    {
        tKeyPressed = false;
    }

    // Cycle Torch Flame Brightness with [G]
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (!gKeyPressed)
        {
            gKeyPressed = true;
            cave.cycleTorchBrightness();
            std::cout << "[FLAMING TORCH] Flame Brightness set to: "
                      << cave.getTorchBrightnessName() << std::endl;
        }
    }
    else
    {
        gKeyPressed = false;
    }

    // Direct key [1] / [Numpad 1]: Night
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
    {
        if (!key1Pressed)
        {
            key1Pressed = true;
            currentTime = TIME_NIGHT;
            std::cout << "[Time of Day] NIGHT mode selected.\n";
        }
    }
    else key1Pressed = false;

    // Direct key [2] / [Numpad 2]: Sunset
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
    {
        if (!key2Pressed)
        {
            key2Pressed = true;
            currentTime = TIME_SUNSET;
            std::cout << "[Time of Day] SUNSET mode selected.\n";
        }
    }
    else key2Pressed = false;

    // Direct key [3] / [Numpad 3]: Day
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
    {
        if (!key3Pressed)
        {
            key3Pressed = true;
            currentTime = TIME_DAY;
            std::cout << "[Time of Day] DAY mode selected.\n";
        }
    }
    else key3Pressed = false;
}
