#version 330 core
in float vTwinkle;
out vec4 FragColor;

uniform float starAlpha; // Global star opacity (fades out at daytime)

void main()
{
    // Convert square point into soft circular star
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;

    // Soft diamond/circular falloff with bright core
    float core = exp(-dist * 6.0);
    float halo = smoothstep(0.5, 0.0, dist) * 0.35;
    float alpha = (core + halo) * vTwinkle * starAlpha;

    vec3 starColor = vec3(0.92, 0.96, 1.0); // Crisp celestial white-blue
    FragColor = vec4(starColor * alpha, alpha);
}
