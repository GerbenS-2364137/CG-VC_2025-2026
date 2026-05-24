#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include "Shader.h"

class ChromaKeyOverlay {
public:
    ChromaKeyOverlay(const std::string& texturePath);
    ~ChromaKeyOverlay();

    void render();

private:
    GLuint VAO, VBO, textureID;
    Shader shader;

    void loadTexture(const std::string& path);
    void initQuad();
};

