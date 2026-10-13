#include "objects/Stars.h"
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Stars::Stars(unsigned int count)
    : VAO(0), VBO(0), starCount(count)
{
    std::vector<float> data;
    data.reserve(count * 6);

    srand(1337); // Deterministic seed for visually balanced star distribution

    for (unsigned int i = 0; i < count; ++i)
    {
        // Spherical distribution on upper dome
        float theta = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI; // Azimuth 0 to 2pi
        float phi   = 0.08f * (float)M_PI + ((float)rand() / RAND_MAX) * 0.42f * (float)M_PI; // Elevation 14 to 90 deg
        float dist  = 115.0f + ((float)rand() / RAND_MAX) * 30.0f; // Distance from center

        float x = dist * std::cos(phi) * std::cos(theta);
        float y = dist * std::sin(phi); // Above horizon
        float z = dist * std::cos(phi) * std::sin(theta);

        float size  = 3.5f + ((float)rand() / RAND_MAX) * 4.0f; // Point size 3.5 to 7.5
        float speed = 1.2f + ((float)rand() / RAND_MAX) * 3.5f; // Twinkle pulse speed
        float phase = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI; // Phase offset

        data.push_back(x);
        data.push_back(y);
        data.push_back(z);
        data.push_back(size);
        data.push_back(speed);
        data.push_back(phase);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    // Attrib 0: Position (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Attrib 1: Params (vec3: size, speed, phase)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Stars::~Stars()
{
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
}

void Stars::draw() const
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, starCount);
    glBindVertexArray(0);
}
