
#ifndef CGVC_PROJECT_OBJECT_H
#define CGVC_PROJECT_OBJECT_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "lichtbron.h"
#include "CameraController.h"

class Object {
    public:
    Object(const std::string& modelPath, const std::string& texturePath = "");
    bool init();
    void render();
    void cleanup();
    bool usesTexture() const { return useTexture; }
    void renderWithLights(Shader& shader, const LightManager& lightManager, const CameraController& CameraController, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

private:
    std::string modelPath;
    std::string texturePath;

    GLuint VAO, VBO, EBO;
    GLuint textureID = 0;

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    bool useTexture = false;

    void loadModel();
    void loadTexture();
};


#endif //CGVC_PROJECT_OBJECT_H