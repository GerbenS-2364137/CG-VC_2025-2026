#include "object.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include <iostream>

Object::Object(const std::string& modelPath, const std::string& texturePath)
    : modelPath(modelPath), texturePath(texturePath) {
}

bool Object::init() {
    loadModel();
    loadTexture();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return true;
}

void Object::loadModel() {
    std::cout << "=== Loading Model: " << modelPath << " ===" << std::endl;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << "\n";
        return;
    }

    // Debug: Scene information
    std::cout << "Scene loaded successfully!" << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;

    if (scene->mNumMeshes == 0) {
        std::cerr << "ERROR: No meshes found in the model!" << std::endl;
        return;
    }

    // Track bounding box for scale analysis
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;

    unsigned int totalVertices = 0;
    unsigned int totalFaces = 0;

    // Loop through ALL meshes instead of just mesh 0
    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        totalVertices += mesh->mNumVertices;
        totalFaces += mesh->mNumFaces;

        // Store the current vertex offset for indices
        unsigned int vertexOffset = vertices.size() / 8;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            // Position
            float x = mesh->mVertices[i].x;
            float y = mesh->mVertices[i].y;
            float z = mesh->mVertices[i].z;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Update bounding box
            minX = std::min(minX, x); maxX = std::max(maxX, x);
            minY = std::min(minY, y); maxY = std::max(maxY, y);
            minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);

            // Normal
            if (mesh->mNormals) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }
            else {
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }

            // TexCoords
            if (mesh->mTextureCoords[0]) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
            else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        // Process faces and adjust indices for the current mesh
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j] + vertexOffset);
            }
        }
    }

    // Debug: Final data information
    std::cout << "=== Final Model Data ===" << std::endl;
    std::cout << "Total vertices from all meshes: " << totalVertices << std::endl;
    std::cout << "Total faces from all meshes: " << totalFaces << std::endl;
    std::cout << "Total vertices loaded: " << vertices.size() / 8 << std::endl;
    std::cout << "Total indices loaded: " << indices.size() << std::endl;
    std::cout << "Total triangles: " << indices.size() / 3 << std::endl;

    // Debug: Bounding box (scale analysis)
    std::cout << "=== Bounding Box ===" << std::endl;
    std::cout << "X: [" << minX << ", " << maxX << "] (size: " << (maxX - minX) << ")" << std::endl;
    std::cout << "Y: [" << minY << ", " << maxY << "] (size: " << (maxY - minY) << ")" << std::endl;
    std::cout << "Z: [" << minZ << ", " << maxZ << "] (size: " << (maxZ - minZ) << ")" << std::endl;

    float maxDimension = std::max({ maxX - minX, maxY - minY, maxZ - minZ });
    std::cout << "Largest dimension: " << maxDimension << std::endl;

    if (maxDimension > 50.0f) {
        std::cout << "WARNING: Model is very large! Consider scaling down by factor: " << (10.0f / maxDimension) << std::endl;
    }
    else if (maxDimension < 0.1f) {
        std::cout << "WARNING: Model is very small! Consider scaling up by factor: " << (1.0f / maxDimension) << std::endl;
    }

    // Debug: Center point
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;
    std::cout << "Model center: (" << centerX << ", " << centerY << ", " << centerZ << ")" << std::endl;

    std::cout << "=== Model Loading Complete ===" << std::endl;
}


void Object::loadTexture()
{
    if (texturePath.empty())
        return;

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;

    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        std::cerr << "Failed to load texture: "
                  << texturePath << std::endl;

        return;
    }

    GLenum format = GL_RGB;

    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glGenTextures(1, &textureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    useTexture = true;
}
void Object::render() {
    if (useTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Object::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

void Object::renderWithLights(Shader& shader, const LightManager& lightManager, const CameraController& CameraController, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model) {
    // Make sure shader is active
    shader.use();

    // Check if shader is valid before setting uniforms
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program == 0) {
        std::cerr << "No shader program is currently active!" << std::endl;
        return;
    }

    // Set uniforms with error checking
    GLenum error = glGetError(); // Clear any previous errors

    shader.setVec3("viewPos", CameraController.getPosition());
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting viewPos uniform: " << error << std::endl;
    }

    // Send light data to shader
    lightManager.sendToShader(shader);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error sending light data to shader: " << error << std::endl;
    }

    shader.setMat4("view", view);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting view matrix: " << error << std::endl;
    }

    shader.setMat4("projection", projection);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting projection matrix: " << error << std::endl;
    }

    shader.setMat4("model", model);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting model matrix: " << error << std::endl;
    }

    shader.setBool("useTexture", usesTexture());
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting useTexture uniform: " << error << std::endl;
    }

    // Render object
    render();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error during vehicle.render(): " << error << std::endl;
    }
}