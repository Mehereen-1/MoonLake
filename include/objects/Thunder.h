#ifndef THUNDER_H
#define THUNDER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Vertex layout for lightning bolt line primitives
struct LightningVertex {
    glm::vec3 pos;
    glm::vec4 color;
};

// ---------------------------------------------------------------------------
// Thunder: Manages atmospheric sky lightning flashes and 3D branching lightning
// bolts that illuminate the sky, clouds, landscape, and lake water during rain.
// ---------------------------------------------------------------------------
class Thunder {
public:
    Thunder();
    ~Thunder();

    // Updates thunder timer and multi-stroke flash envelope.
    // If isRaining is true, triggers natural periodic slight thunder in the sky.
    // If isRaining is false, immediately resets and turns off.
    void update(float dt, bool isRaining);

    // Manually trigger a thunder strike in the sky
    void triggerStrike();

    // Renders the visible 3D branching lightning bolt in the sky
    void draw() const;

    // Current atmospheric flash illumination factor [0.0f, 1.0f]
    float getFlashIntensity() const { return m_flashIntensity; }

    // Whether a lightning bolt is actively flashing in the sky
    bool isFlashing() const { return m_isFlashing; }

private:
    void generateLightningBolt();

    unsigned int m_VAO;
    unsigned int m_VBO;
    std::vector<LightningVertex> m_vertices;

    bool m_isFlashing;
    float m_flashTimer;
    float m_flashDuration;
    float m_flashIntensity;
    float m_timeUntilNextStrike;
    bool m_wasRaining;
};

#endif // THUNDER_H
