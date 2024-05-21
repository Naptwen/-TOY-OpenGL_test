#version 330 core

layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 LightDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos; // Light position

void main()
{
    vec4 worldPos = model * vec4(mPos, 1.0);
    gl_Position = projection * view * worldPos;
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model * view))) * mNormal;
    LightDir = normalize(lightPos - FragPos); // Direction to the light source
}