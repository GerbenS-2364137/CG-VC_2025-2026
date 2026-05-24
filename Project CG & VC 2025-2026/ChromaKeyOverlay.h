#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include "shader.h"

class ChromaKeyOverlay {
public:
    ChromaKeyOverlay(const std::string& texturePath);
    ~ChromaKeyOverlay();

    void render();

    // Pas de keyingkleur aan (Cb, Cr in YCbCr-ruimte)
    void setChromaKey(glm::vec2 cbcr) { chromaKey = cbcr; }
    void setThreshold(float t) { threshold = t; }

private:
    GLuint VAO, VBO, textureID;
    Shader shader;
    // Standaard: groen (0.169, 0.500)
    glm::vec2 chromaKey = glm::vec2(0.169f, 0.500f);
    float threshold = 0.25f;

    void loadTexture(const std::string& path);
    void initQuad();
};

