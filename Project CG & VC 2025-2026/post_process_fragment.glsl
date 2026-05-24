#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform int effect;
uniform vec3 texelSize;

// Edge detection kernel
const float edgeKernel[9] = float[](
    -1, -1, -1,
    -1, 8, -1,
    -1, -1, -1
    );

// Simple blur kernel
const float blurKernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
    );

vec3 applyKernel(float kernel[9]) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            vec2 offset = vec2((i - 1) * texelSize.x, (j - 1) * texelSize.y);
            result += texture(screenTexture, TexCoord + offset).rgb * kernel[i * 3 + j];
        }
    }

    return result;
}

void main() {
    vec3 color = texture(screenTexture, TexCoord).rgb;

    if (effect == 0) {
        // No effect
        FragColor = vec4(color, 1.0);
    }
    else if (effect == 1) {
        // Gaussian blur
        color = applyKernel(blurKernel);
        FragColor = vec4(color, 1.0);
    }
    else if (effect == 2) {
        // Edge detection
        color = applyKernel(edgeKernel);
        FragColor = vec4(color, 1.0);
    }
    else {
        // Default: no effect
        FragColor = vec4(color, 1.0);
    }
}