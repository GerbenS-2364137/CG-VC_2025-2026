#version 330 core

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 32

uniform PointLight lights[MAX_LIGHTS];
uniform int numLights;

uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 objectColor) {
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

    // Attenuation (afname met afstand)
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine components
    vec3 ambient = 0.05 * objectColor;
    vec3 diffuse = diff * light.color * objectColor;
    vec3 specular = spec * light.color * 0.5; // Specular intensity

    // Apply attenuation and intensity
    ambient *= attenuation * light.intensity;
    diffuse *= attenuation * light.intensity;
    specular *= attenuation * light.intensity;

    return ambient + diffuse + specular;
}

void main() {
    vec3 color;

    if (useTexture) {
        color = texture(texture1, TexCoord).rgb;
    }
    else {
        // Behoud de originele verticale gradient van blauw naar rood
        vec3 colorBottom = vec3(0.0, 0.0, 1.0); // blauw
        vec3 colorTop = vec3(1.0, 0.0, 0.0);    // rood
        color = mix(colorBottom, colorTop, TexCoord.y);
    }

    // Alleen lighting toepassen als er normalen zijn
    vec3 result;
    if (length(Normal) > 0.01) { // Check of we geldige normalen hebben
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);

        result = vec3(0.0);

        // Bereken bijdrage van alle lichten
        for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
            result += calculatePointLight(lights[i], norm, FragPos, viewDir, color);
        }

        // Als er geen lichten zijn, gebruik basis ambient lighting
        if (numLights == 0) {
            result = color * 0.3;
        }
    }
    else {
        // Fallback voor objecten zonder normalen - gebruik originele kleur
        result = color;
    }

    FragColor = vec4(result, 1.0);
}