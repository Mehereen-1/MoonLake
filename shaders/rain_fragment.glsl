#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform float rainAlpha;

void main()
{
    FragColor = vec4(vColor.rgb, vColor.a * rainAlpha);
}
