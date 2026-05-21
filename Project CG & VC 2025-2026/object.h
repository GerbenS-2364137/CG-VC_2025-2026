
#ifndef CGVC_PROJECT_OBJECT_H
#define CGVC_PROJECT_OBJECT_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class object {
    public:
    object(const std::string& modelPath, const std::string& texturePath = "");
    bool init();
    void render();
    void cleanup();
    bool usesTexture() const { return useTexture; }

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