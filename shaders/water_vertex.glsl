#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 vertexColor;
out vec2 LakeUV;
out float ShoreDist;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 lakeCenter;
uniform vec2 lakeRadius;

// Directional Gerstner wave parameters (Direction, Amplitude, Wavelength, Speed, Steepness)
struct GerstnerWave {
    vec2 dir;
    float amp;
    float len;
    float speed;
    float steepness;
};

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    // Lake shoreline distance factor (0.0 in center, 1.0 at shore perimeter)
    vec2 rel = (worldPos.xz - lakeCenter.xz) / lakeRadius;
    float distToShore = length(rel);
    ShoreDist = distToShore;
    LakeUV = rel * 0.5 + 0.5;

    // Dampen waves near the shoreline (water is naturally calmer near shallow banks)
    float shoreDamp = clamp(1.0 - smoothstep(0.72, 1.02, distToShore), 0.20, 1.0);

    // 4 Directional Gerstner Waves (Gentle natural mountain lake ripples)
    GerstnerWave waves[4] = GerstnerWave[4](
        GerstnerWave(normalize(vec2( 0.70,  0.40)), 0.009, 4.2, 0.75, 0.15),
        GerstnerWave(normalize(vec2(-0.45,  0.80)), 0.006, 2.8, 0.90, 0.12),
        GerstnerWave(normalize(vec2( 0.30, -0.90)), 0.004, 1.8, 1.10, 0.10),
        GerstnerWave(normalize(vec2(-0.85, -0.35)), 0.0025, 1.1, 1.30, 0.08)
    );

    vec3 displacedPos = worldPos.xyz;

    float dX_accum = 0.0;
    float dZ_accum = 0.0;
    float dY_accum = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        float k = 6.283185 / waves[i].len; // wave number
        float c = waves[i].speed;          // phase speed
        float a = waves[i].amp * shoreDamp;
        float q = waves[i].steepness / (k * a * 4.0 + 0.0001);
        q = clamp(q, 0.0, 0.55);

        float phase = k * (dot(waves[i].dir, worldPos.xz) - c * time);
        float sinP = sin(phase);
        float cosP = cos(phase);

        // Gerstner vertex displacement (sharp crests, rounded troughs)
        displacedPos.x += q * a * waves[i].dir.x * cosP;
        displacedPos.y += a * sinP;
        displacedPos.z += q * a * waves[i].dir.y * cosP;

        // Analytical normal derivatives
        float wa = k * a;
        dX_accum += waves[i].dir.x * wa * cosP;
        dZ_accum += waves[i].dir.y * wa * cosP;
        dY_accum += q * wa * sinP;
    }

    vec3 waveNormal = normalize(vec3(-dX_accum, 1.0 - dY_accum, -dZ_accum));

    FragPos = displacedPos;
    Normal = waveNormal;
    vertexColor = aColor;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
