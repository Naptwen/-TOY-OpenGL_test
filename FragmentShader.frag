#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 LightDir;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, LightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular BRDF equation
    vec3 viewDir = normalize(-FragPos); // Viwer direction
    vec3 halfwayDir = normalize(LightDir + viewDir); // Halfway direction between light and camera
    float angle = max(dot(norm, halfwayDir), 0.0); // get cos of angle between normal and halfway direction
    float spec = pow(angle, 15.0); // get specular strength
    vec3 specular = spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}