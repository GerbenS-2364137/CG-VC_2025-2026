#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float threshold;

void main() {
    vec3 color = texture(screenTexture, TexCoord).rgb;

    // Calculate brightness using luminance formula
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Only output pixels above threshold
    if (brightness > threshold) {
        FragColor = vec4(color, 1.0);
    }
    else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}