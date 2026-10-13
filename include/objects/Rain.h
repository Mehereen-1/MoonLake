#ifndef RAIN_H
#define RAIN_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// ---------------------------------------------------------------------------
// Raindrop: Represents an individual physical raindrop falling through the scene
// ---------------------------------------------------------------------------
struct Raindrop {
    glm::vec3 pos;       // Current world position (impact tip)
    float speed;         // Falling velocity (e.g. 21.0f - 27.0f m/s)
    float length;        // Streak length (e.g. 0.45f - 0.70f m)
    float alpha;         // Base transparency (e.g. 0.40f - 0.70f)
    float resetHeight;   // High respawn altitude (e.g. 18.0f - 23.0f m)
    float groundHeight;  // Collision floor altitude (e.g. -1.2f m)
};

// Vertex layout for rain streak line primitives
struct RainVertex {
    glm::vec3 pos;
    glm::vec4 color;
};

// ---------------------------------------------------------------------------
// Rain: Manages GPU dynamic buffers and rendering for thousands of continuous
// falling raindrops across the scene using high-performance GL_LINES.
// ---------------------------------------------------------------------------
class Rain {
public:
    // Full world boundaries covering the entire landscape (-85m to +85m, -60m to +130m)
    static constexpr float BOUNDS_MIN_X = -90.0f;
    static constexpr float BOUNDS_MAX_X =  90.0f;
    static constexpr float BOUNDS_MIN_Z = -65.0f;
    static constexpr float BOUNDS_MAX_Z = 135.0f;

    // Initializes particle pool across the entire environment volume
    Rain(int dropCount = 18000);
    ~Rain();

    // Updates the dynamic GPU buffer with the latest streak positions
    void updateBuffers(const glm::vec3& fallDir);

    // Renders the rain streaks
    void draw() const;

    // Accessors
    std::vector<Raindrop>& getDrops() { return m_drops; }
    const std::vector<Raindrop>& getDrops() const { return m_drops; }
    int getDropCount() const { return (int)m_drops.size(); }

    // Evaluates ground / obstacle collision height at (x, z)
    static float getGroundHeight(float x, float z);

private:
    unsigned int m_VAO;
    unsigned int m_VBO;
    std::vector<Raindrop> m_drops;
    std::vector<RainVertex> m_vertexCache;
};

#endif // RAIN_H
