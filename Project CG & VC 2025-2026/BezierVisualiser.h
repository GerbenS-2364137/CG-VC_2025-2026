#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "Bezier.h"
#include "object.h"
#include "shader.h"
#include "lichtbron.h" 

struct ObjectInstance {
    glm::vec3 position;
    glm::vec3 tangent;
    glm::mat4 transformMatrix;
};

class BezierVisualiser {
public:
    BezierVisualiser();
    ~BezierVisualiser();

    void init(const Bezier::BezierCurve& curve, const std::string& objectPath, const std::string& texturePath);
    void render(const glm::mat4& view, const glm::mat4& projection, const LightManager& lightManager, const glm::vec3& viewPos);
    void cleanup();

    // Configuration methods
    void setObjectSpacing(float spacing) { objectSpacing = spacing; }
    void setZSquishFactor(float factor) { zSquishFactor = factor; }
    void setObjectScale(float scale) { objectScale = scale; }

private:
    std::unique_ptr<Object> pathObject;
    std::vector<ObjectInstance> instances;
    Shader objectShader;
    glm::vec3 objectCenter;

    // Configuration parameters
    float objectSpacing = 0.14f;
    float zSquishFactor = 0.14f;
    float objectScale = 0.7f;

    void generateInstances(const Bezier::BezierCurve& curve);
    glm::mat4 calculateTransformMatrix(const glm::vec3& position, const glm::vec3& tangent);
    void calculateObjectCenter();
};