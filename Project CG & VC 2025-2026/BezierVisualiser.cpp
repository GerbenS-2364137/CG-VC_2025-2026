#include "BezierVisualiser.h"
#include <glad/glad.h>
#include <iostream>

BezierVisualiser::BezierVisualiser()
    : objectShader("vertex_shader.glsl", "fragment_shader.glsl") {
}

BezierVisualiser::~BezierVisualiser() {
    cleanup();
}

void BezierVisualiser::init(const Bezier::BezierCurve& curve, const std::string& objectPath, const std::string& texturePath) {
    // Load the object that will be repeated along the curve
    pathObject = std::make_unique<Object>(objectPath, texturePath);

    if (!pathObject->init()) {
        std::cerr << "Failed to initialize path object: " << objectPath << std::endl;
        return;
    }

    std::cout << "Loaded path object: " << objectPath << std::endl;

    // Calculate the object's center for proper positioning
    calculateObjectCenter();

    // Generate instances along the curve
    generateInstances(curve);

    if (!objectShader.isValid()) {
        std::cerr << objectShader.getInfoLog() << std::endl;
    }
}

void BezierVisualiser::generateInstances(const Bezier::BezierCurve& curve) {
    instances.clear();

    auto pts = curve.forwardDifferencing(100);
    float curveLength = 0.0f;
    for (size_t i = 1; i < pts.size(); ++i)
        curveLength += glm::length(pts[i] - pts[i - 1]);

    // Calculate number of objects based on spacing
    int numObjects = static_cast<int>(curveLength / objectSpacing);
    if (numObjects < 2) numObjects = 2; // Minimum 2 objects

    std::cout << "Generating " << numObjects << " objects along curve (length: " << curveLength << ")" << std::endl;

    // Generate instances at evenly spaced intervals
    for (int i = 0; i < numObjects; ++i) {
        float t = static_cast<float>(i) / (numObjects - 1);

        ObjectInstance instance;
        instance.position = curve.evaluate(t);
        instance.tangent = curve.tangent(t);

        // Generate transform matrix
        instance.transformMatrix = calculateTransformMatrix(instance.position, instance.tangent);

        instances.push_back(instance);
    }
}

glm::mat4 BezierVisualiser::calculateTransformMatrix(const glm::vec3& position, const glm::vec3& tangent) {
    // Create transformation matrix: Translation * Rotation * Scale * Centering

    // 1. Center the object (move from object's pivot to its center)
    glm::mat4 centering = glm::translate(glm::mat4(1.0f), -objectCenter);

    // 2. Scale (with Z-axis squishing) - applied after centering
    glm::vec3 scaleVec(objectScale, objectScale, objectScale * zSquishFactor);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleVec);

    // 3. Rotation to align with tangent
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    // Handle case where tangent is nearly vertical
    if (glm::abs(glm::dot(tangent, up)) > 0.95f) {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Create orthonormal basis
    glm::vec3 right = glm::normalize(glm::cross(up, tangent));
    glm::vec3 correctedUp = glm::cross(tangent, right);

    // Build rotation matrix (TBN frame)
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(correctedUp, 0.0f);
    rotation[2] = glm::vec4(tangent, 0.0f);

    // 4. Translation to final position
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);

    // Combine transformations: T * R * S * C
    return translation * rotation * scale * centering;
}

void BezierVisualiser::render(const glm::mat4& view, const glm::mat4& projection, const LightManager& lightManager, const glm::vec3& viewPos) {
    if (!pathObject) {
        return;
    }

    objectShader.use();
    objectShader.setMat4("view", view);
    objectShader.setMat4("projection", projection);
    objectShader.setBool("useTexture", pathObject->usesTexture());

    // Set camera position for lighting calculations
    objectShader.setVec3("viewPos", viewPos);

    // Send light data to the shader
    lightManager.sendToShader(objectShader);

    // Try flipping normals for road objects - remove this if it doesn't help
    objectShader.setBool("flipNormals", true);

    // Render each instance
    for (const auto& instance : instances) {
        objectShader.setMat4("model", instance.transformMatrix);
        pathObject->render();
    }

    if (!objectShader.isValid()) {
        std::cerr << objectShader.getInfoLog() << std::endl;
    }
}

void BezierVisualiser::cleanup() {
    if (pathObject) {
        pathObject->cleanup();
        pathObject.reset();
    }
    instances.clear();
}

void BezierVisualiser::calculateObjectCenter() {

    objectCenter = glm::vec3(0.0, 0.0, 0.0f);

    std::cout << "Object center calculated as: ("
        << objectCenter.x << ", " << objectCenter.y << ", " << objectCenter.z << ")" << std::endl;
}