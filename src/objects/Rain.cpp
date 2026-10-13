#include "objects/Rain.h"
#include "objects/TerrainMesh.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Helper pseudo-random function for initial setup
static float pseudoRand(int seed)
{
    float f = std::sin((float)seed * 12.9898f + 78.233f) * 43758.5453f;
    return f - std::floor(f);
}

float Rain::getGroundHeight(float x, float z)
{
    // 1. Cabin Roof & Porch (Rain drops strike the wooden roof at X = -12.5m, Z = -2.0m)
    if (x > -14.8f && x < -10.2f && z > -4.2f && z < 0.2f)
    {
        return 3.2f;
    }

    // 2. Eastern Mountain & Mystical Cave Rock Mass (Rain drops strike mountain top)
    if (x > 24.0f && z > -14.0f && z < 2.0f)
    {
        return 4.5f;
    }

    // 3. Lakeside Windmill Structure (X = 7.5m, Z = -27.5m)
    if (std::abs(x - 7.5f) < 2.5f && std::abs(z - (-27.5f)) < 2.5f)
    {
        return 5.5f;
    }

    // 4. Full scene terrain and lake water level
    bool dummyValid = true;
    float elev = TerrainMesh::computeElevation(x, z, dummyValid);
    return std::max(-1.18f, elev);
}

Rain::Rain(int dropCount)
    : m_VAO(0), m_VBO(0)
{
    m_drops.reserve(dropCount);

    const float minX = BOUNDS_MIN_X;
    const float maxX = BOUNDS_MAX_X;
    const float minZ = BOUNDS_MIN_Z;
    const float maxZ = BOUNDS_MAX_Z;

    for (int i = 0; i < dropCount; ++i)
    {
        Raindrop drop;

        // Spread raindrops across entire visible scene
        float rx = pseudoRand(i * 3 + 1);
        float rz = pseudoRand(i * 7 + 2);
        float ry = pseudoRand(i * 13 + 5);

        drop.pos.x = minX + rx * (maxX - minX);
        drop.pos.z = minZ + rz * (maxZ - minZ);

        // Ground / collision altitude for this X, Z
        drop.groundHeight = getGroundHeight(drop.pos.x, drop.pos.z);

        // High respawn altitude up near cloud base
        drop.resetHeight = 24.0f + pseudoRand(i * 17 + 3) * 6.0f;

        // Distribute initial heights continuously between ground and clouds
        drop.pos.y = drop.groundHeight + ry * (drop.resetHeight - drop.groundHeight);

        // Natural speed variation: 21.0 to 26.5 m/s
        drop.speed = 21.0f + pseudoRand(i * 23 + 4) * 5.5f;

        // Slender streak length: 0.45 to 0.70 meters
        drop.length = 0.45f + pseudoRand(i * 31 + 6) * 0.25f;

        // Soft translucency: 0.40 to 0.70
        drop.alpha = 0.40f + pseudoRand(i * 41 + 8) * 0.30f;

        m_drops.push_back(drop);
    }

    m_vertexCache.resize(dropCount * 2);

    // Setup dynamic GPU buffer
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, dropCount * 2 * sizeof(RainVertex), nullptr, GL_DYNAMIC_DRAW);

    // Layout 0: Position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RainVertex), (void*)offsetof(RainVertex, pos));

    // Layout 1: Color + Alpha (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RainVertex), (void*)offsetof(RainVertex, color));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Rain::~Rain()
{
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void Rain::updateBuffers(const glm::vec3& fallDir)
{
    const glm::vec3 normDir = glm::normalize(fallDir);
    const size_t count = m_drops.size();

    for (size_t i = 0; i < count; ++i)
    {
        const Raindrop& d = m_drops[i];

        // 1. Drop Head / Impact Tip: Bright silvery-cyan glistening drop
        m_vertexCache[i * 2 + 0].pos = d.pos;
        m_vertexCache[i * 2 + 0].color = glm::vec4(0.80f, 0.90f, 0.98f, d.alpha);

        // 2. Drop Tail / Motion Trail: Soft tapered translucent streak extending along slant vector
        m_vertexCache[i * 2 + 1].pos = d.pos - normDir * d.length;
        m_vertexCache[i * 2 + 1].color = glm::vec4(0.55f, 0.70f, 0.88f, d.alpha * 0.12f);
    }

    // Stream updated streak vertices to the GPU dynamic buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexCache.size() * sizeof(RainVertex), m_vertexCache.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Rain::draw() const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Rain does not occlude objects behind it
    glLineWidth(1.4f);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)(m_drops.size() * 2));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}
