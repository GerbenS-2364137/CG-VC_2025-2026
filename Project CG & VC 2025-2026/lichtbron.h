#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "shader.h"

struct lichtbron {
    glm::vec3 position;
    glm::vec3 color;

    // Attenuation parameters
    float constant;     // Constante term
    float linear;       // Lineaire term
    float quadratic;    // Kwadratische term

    // Light intensity
    float intensity;

    lichtbron(
        glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 col = glm::vec3(1.0f),
        float intens = 1.0f,
        float cons = 1.0f,
        float lin = 0.045f,
        float quad = 0.0075f)
        : position(pos), color(col), intensity(intens),
        constant(cons), linear(lin), quadratic(quad) {
    }
};

class LightManager {
public:
    LightManager() = default;

    // Voeg een licht toe
    void addLight(const lichtbron& light);

    // Verwijder alle lichten
    void clearLights();

    // Update licht op specifieke index
    void updateLight(int index, const lichtbron& light);

    // Haal een licht op
    lichtbron& getLight(int index);
    const lichtbron& getLight(int index) const;

    // Aantal lichten
    int getLightCount() const { return static_cast<int>(lights.size()); }

    // Send light data to shader
    void sendToShader(Shader& shader) const;

    // Presets
    static lichtbron createBrightLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f));
    static lichtbron createDistantLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f));
    static lichtbron createCloseLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f));


private:
    std::vector<lichtbron> lights;
    static const int MAX_LIGHTS = 32;
};

