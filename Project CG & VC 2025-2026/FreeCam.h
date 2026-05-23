#pragma once
#include "Camera.h"

class FreeCamera : public Camera {
public:
    FreeCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f,
        float zoom = 45.0f)
        : Camera(position, up, yaw, pitch, zoom) {
    }
};
