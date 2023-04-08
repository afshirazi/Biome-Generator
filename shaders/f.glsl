#version 330 core

in vec3 outFragCol;

out vec4 FragColor;

void main()
{
    FragColor = vec4(outFragCol.x, outFragCol.y, outFragCol.z, 1.0f);
} 