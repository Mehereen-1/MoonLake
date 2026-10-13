#version 330 core
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 skyHorizonColor;
uniform vec3 skyZenithColor;
uniform float horizonY;
uniform float zenithY;

void main()
{
    float hY = horizonY;
    float zY = zenithY > horizonY ? zenithY : (horizonY + 25.0);

    // Smooth nonlinear atmospheric gradient
    float factor = clamp((FragPos.y - hY) / (zY - hY), 0.0, 1.0);
    factor = pow(factor, 0.75); // Gives a slightly broader, softer horizon glow

    vec3 skyColor = mix(skyHorizonColor, skyZenithColor, factor);
    FragColor = vec4(skyColor, 1.0);
}
