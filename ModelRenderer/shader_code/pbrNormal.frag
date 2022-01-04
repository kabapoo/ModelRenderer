#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform mat4 projection;
uniform mat4 normal_view;
uniform mat4 model;

// ----------------------------------------------------------------------------
void main()
{
    //vec3 normal = Normal * 0.5f + 0.5f;
    FragColor = normal_view * vec4(Normal, 1.0) * 0.5f + 0.5f;
}