#include "objects/Thunder.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

Thunder::Thunder()
    : m_VAO(0),
      m_VBO(0),
      m_isFlashing(false),
      m_flashTimer(0.0f),
      m_flashDuration(0.55f),
      m_flashIntensity(0.0f),
      m_timeUntilNextStrike(3.0f),
      m_wasRaining(false)
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // Preallocate buffer for up to 1024 vertices (plenty for main trunk + branch forks)
    glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(LightningVertex), nullptr, GL_DYNAMIC_DRAW);

    // Layout 0: Position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LightningVertex), (void*)offsetof(LightningVertex, pos));

    // Layout 1: Color + Alpha (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LightningVertex), (void*)offsetof(LightningVertex, color));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Thunder::~Thunder()
{
    if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
    if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
}

void Thunder::triggerStrike()
{
    m_isFlashing = true;
    m_flashTimer = 0.0f;
    m_flashDuration = 0.48f + ((float)(rand() % 1000) / 1000.0f) * 0.18f;
    m_flashIntensity = 0.85f;

    generateLightningBolt();

    std::cout << "[Thunder] Distant lightning flash illuminating the sky!\n";
}

void Thunder::update(float dt, bool isRaining)
{
    if (!isRaining)
    {
        m_isFlashing = false;
        m_flashIntensity = 0.0f;
        m_flashTimer = 0.0f;
        m_timeUntilNextStrike = 3.0f;
        m_wasRaining = false;
        return;
    }

    // When rain first turns on, schedule initial thunder strike after a brief delay
    if (!m_wasRaining && isRaining)
    {
        m_wasRaining = true;
        m_timeUntilNextStrike = 2.5f + ((float)(rand() % 1000) / 1000.0f) * 1.5f;
    }

    if (m_isFlashing)
    {
        m_flashTimer += dt;
        float progress = m_flashTimer / m_flashDuration;

        if (progress >= 1.0f)
        {
            m_isFlashing = false;
            m_flashIntensity = 0.0f;
            // Schedule next periodic strike between 7.0 and 13.0 seconds
            m_timeUntilNextStrike = 7.0f + ((float)(rand() % 1000) / 1000.0f) * 6.0f;
        }
        else
        {
            // Natural multi-stroke lightning flash signature
            // Phase 1: rapid primary stroke (0.0 to 0.12)
            // Phase 2: brief dip between strokes (0.12 to 0.22)
            // Phase 3: main return stroke surge (0.22 to 0.38)
            // Phase 4: smooth quadratic dissipation afterglow (0.38 to 1.0)
            float rawIntensity = 0.0f;
            if (progress < 0.12f)
            {
                float p = progress / 0.12f;
                rawIntensity = std::sin(p * 1.5707963f);
            }
            else if (progress < 0.22f)
            {
                float p = (progress - 0.12f) / 0.10f;
                rawIntensity = 1.0f - 0.70f * p;
            }
            else if (progress < 0.38f)
            {
                float p = (progress - 0.22f) / 0.16f;
                rawIntensity = 0.30f + 0.70f * std::sin(p * 3.14159265f);
            }
            else
            {
                float p = (progress - 0.38f) / 0.62f;
                rawIntensity = std::max(0.0f, (1.0f - p) * (1.0f - p));
            }

            // Calibrated for slight, aesthetic lightning ambience
            m_flashIntensity = rawIntensity * 0.85f;
        }
    }
    else
    {
        m_timeUntilNextStrike -= dt;
        if (m_timeUntilNextStrike <= 0.0f)
        {
            triggerStrike();
        }
    }
}

void Thunder::generateLightningBolt()
{
    m_vertices.clear();

    // 1. Pick a scenic direction in the distant sky across the northern / eastern / western mountains
    float angle = -0.5f + (((float)(rand() % 1000) / 1000.0f) - 0.5f) * 2.6f;
    float dist = 52.0f + ((float)(rand() % 1000) / 1000.0f) * 22.0f;

    glm::vec3 startPos;
    startPos.x = dist * std::sin(angle);
    startPos.z = -dist * std::cos(angle) - 8.0f;
    startPos.y = 32.0f + ((float)(rand() % 1000) / 1000.0f) * 8.0f; // High in upper clouds

    glm::vec3 endPos;
    endPos.x = startPos.x + (((float)(rand() % 1000) / 1000.0f) - 0.5f) * 16.0f;
    endPos.z = startPos.z + (((float)(rand() % 1000) / 1000.0f) - 0.5f) * 10.0f;
    endPos.y = 11.0f + ((float)(rand() % 1000) / 1000.0f) * 6.0f;  // Lower cloud / ridge horizon

    const int numSegments = 16;
    std::vector<glm::vec3> trunkPoints;
    trunkPoints.reserve(numSegments + 1);
    trunkPoints.push_back(startPos);

    // 2. Generate jagged main trunk path
    for (int i = 1; i < numSegments; ++i)
    {
        float t = (float)i / (float)numSegments;
        glm::vec3 interp = glm::mix(startPos, endPos, t);

        // Transverse jagged displacement
        float envelope = std::sin(t * 3.14159265f);
        float jx = (((float)(rand() % 1000) / 500.0f) - 1.0f) * (1.8f * envelope);
        float jy = (((float)(rand() % 1000) / 500.0f) - 1.0f) * (0.6f * envelope);
        float jz = (((float)(rand() % 1000) / 500.0f) - 1.0f) * (1.8f * envelope);

        trunkPoints.push_back(interp + glm::vec3(jx, jy, jz));
    }
    trunkPoints.push_back(endPos);

    // 3. Populate line primitives for main trunk (radiant electric white-cyan core + soft halo)
    const glm::vec4 coreColor(0.96f, 0.98f, 1.0f, 1.0f);
    const glm::vec4 haloColor(0.65f, 0.78f, 1.0f, 0.40f);

    for (size_t i = 0; i + 1 < trunkPoints.size(); ++i)
    {
        // Primary core line
        m_vertices.push_back({ trunkPoints[i], coreColor });
        m_vertices.push_back({ trunkPoints[i + 1], coreColor });

        // Offset halo line for subtle glow width
        glm::vec3 offset(0.08f, 0.0f, 0.08f);
        m_vertices.push_back({ trunkPoints[i] + offset, haloColor });
        m_vertices.push_back({ trunkPoints[i + 1] + offset, haloColor });
    }

    // 4. Generate 2 to 3 secondary branching forks stemming from trunk
    int forkIndices[3] = { 4, 8, 12 };
    for (int k = 0; k < 3; ++k)
    {
        int idx = forkIndices[k];
        if (idx >= (int)trunkPoints.size()) continue;

        glm::vec3 forkOrigin = trunkPoints[idx];
        glm::vec3 branchDir = glm::normalize(endPos - startPos);
        // Diverge sideways
        float divergeAngle = ((k % 2 == 0) ? 0.75f : -0.75f) + (((float)(rand() % 1000) / 1000.0f) - 0.5f) * 0.4f;
        glm::vec3 sideDir(-branchDir.z, 0.0f, branchDir.x);
        glm::vec3 forkDir = glm::normalize(branchDir * 0.65f + sideDir * divergeAngle);

        const int branchSteps = 5;
        glm::vec3 prevPoint = forkOrigin;
        const glm::vec4 branchColor(0.72f, 0.84f, 1.0f, 0.70f);

        for (int b = 1; b <= branchSteps; ++b)
        {
            float stepLen = 1.6f + ((float)(rand() % 1000) / 1000.0f) * 0.8f;
            float bjX = (((float)(rand() % 1000) / 500.0f) - 1.0f) * 0.8f;
            float bjY = (((float)(rand() % 1000) / 500.0f) - 1.0f) * 0.5f - 0.4f;
            float bjZ = (((float)(rand() % 1000) / 500.0f) - 1.0f) * 0.8f;

            glm::vec3 nextPoint = prevPoint + forkDir * stepLen + glm::vec3(bjX, bjY, bjZ);

            m_vertices.push_back({ prevPoint, branchColor });
            m_vertices.push_back({ nextPoint, branchColor });

            prevPoint = nextPoint;
        }
    }

    // 5. Upload lightning bolt line vertices to dynamic GPU buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(LightningVertex), m_vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Thunder::draw() const
{
    if (!m_isFlashing || m_vertices.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for electric plasma
    glDepthMask(GL_FALSE);
    glLineWidth(2.4f);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)m_vertices.size());
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
