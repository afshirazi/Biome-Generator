#version 330 core

in vec3 triCol;

out vec4 FragColor;

void main()
{
    FragColor = vec4(triCol, 1.0f);
} 