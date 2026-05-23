#version 330 core
layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal (voor lighting)
layout(location = 2) in vec2 aTexCoord; // Texture coordinate

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool flipNormals;

out vec3 FragPos;   // World space position voor lighting
out vec3 Normal;    // World space normal voor lighting
out vec2 TexCoord;  // Texture coordinate

void main() {
    // Bereken world space positie
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform normal to world space (belangrijk voor correcte lighting)
    vec3 transformedNormal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Flip normals if requested
    if (flipNormals) {
        transformedNormal = -transformedNormal;
    }

    Normal = transformedNormal;

    // Pass texture coordinate door
    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}