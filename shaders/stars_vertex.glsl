#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aParams; // x: size, y: speed, z: phase

out float vTwinkle;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    float twinkle = 0.50 + 0.50 * sin(time * aParams.y + aParams.z);
    vTwinkle = twinkle;

    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aParams.x * (0.65 + 0.35 * twinkle);
}
