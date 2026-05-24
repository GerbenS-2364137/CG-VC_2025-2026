#include "lichtbron.h"
#include <iostream>
#include <algorithm>

void LightManager::addLight(const lichtbron& light) {
    if (lights.size() < MAX_LIGHTS) {
        lights.push_back(light);
    }
    else {
        std::cerr << "Warning: Maximum number of lights (" << MAX_LIGHTS << ") reached!" << std::endl;
    }
}

void LightManager::clearLights() {
    lights.clear();
}

void LightManager::updateLight(int index, const lichtbron& light) {
    if (index >= 0 && index < static_cast<int>(lights.size())) {
        lights[index] = light;
    }
    else {
        std::cerr << "Warning: Light index " << index << " out of bounds!" << std::endl;
    }
}

lichtbron& LightManager::getLight(int index) {
    static lichtbron dummy; // Fallback voor invalid indices
    if (index >= 0 && index < static_cast<int>(lights.size())) {
        return lights[index];
    }
    std::cerr << "Warning: Light index " << index << " out of bounds!" << std::endl;
    return dummy;
}

const lichtbron& LightManager::getLight(int index) const {
    static lichtbron dummy;
    if (index >= 0 && index < static_cast<int>(lights.size())) {
        return lights[index];
    }
    std::cerr << "Warning: Light index " << index << " out of bounds!" << std::endl;
    return dummy;
}

void LightManager::sendToShader(Shader& shader) const {
    shader.use();

    // Send aantal lichten
    shader.setInt("numLights", getLightCount());

    // Send alle light data
    for (int i = 0; i < getLightCount(); ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";

        shader.setVec3(base + ".position", lights[i].position);
        shader.setVec3(base + ".color", lights[i].color);
        shader.setFloat(base + ".intensity", lights[i].intensity);
        shader.setFloat(base + ".constant", lights[i].constant);
        shader.setFloat(base + ".linear", lights[i].linear);
        shader.setFloat(base + ".quadratic", lights[i].quadratic);
    }
}

// Preset lichten met verschillende eigenschappen
lichtbron LightManager::createBrightLight(glm::vec3 position, glm::vec3 color) {
    // Helder licht dat ver reikt
    return lichtbron(position, color, 2.0f, 1.0f, 0.022f, 0.0019f);
}

lichtbron LightManager::createDistantLight(glm::vec3 position, glm::vec3 color) {
    // Zwakker licht voor achtergrondverlichting
    return lichtbron(position, color, 0.8f, 1.0f, 0.045f, 0.0075f);
}

lichtbron LightManager::createCloseLight(glm::vec3 position, glm::vec3 color) {
    // Sterk licht met snelle afname - goed voor spots
    return lichtbron(position, color, 3.0f, 1.0f, 0.14f, 0.07f);
}