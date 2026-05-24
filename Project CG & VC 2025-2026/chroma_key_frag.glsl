#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D chromaTexture;

void main()
{
    vec4 texColor = texture(chromaTexture, TexCoords);
    
    float threshold = 1.0; // Hoeveel van de kleur toegelaten wordt. 0.0 is alles, 1.0 is niets
    vec3 chromaColor = vec3(0.0, 1.0, 0.0); // Groen

    float diff = distance(texColor.rgb, chromaColor);

    if (diff < threshold)
        discard; // Transparant maken (wegdoen)

    FragColor = texColor;
}