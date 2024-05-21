#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 LightDir;

out vec4 FragColor;

uniform vec3 lightColor; // Light color

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, LightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * vec3(1.0, 1.0, 1.0);
    FragColor = vec4(result, 1.0);
}