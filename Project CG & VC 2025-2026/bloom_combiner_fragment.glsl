#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float bloomIntensity;

void main() {
    vec3 sceneColor = texture(scene, TexCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoord).rgb;

    // Additive blending
    vec3 result = sceneColor + bloomColor * bloomIntensity;

    // Optional tone mapping to prevent over-brightness
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}