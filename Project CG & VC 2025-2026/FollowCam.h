#pragma once
#include "Camera.h"

class FollowCamera : public Camera {
public:
    FollowCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f,
        float zoom = 45.0f)
        : Camera(position, up, yaw, pitch, zoom) {
    }

    void SetTarget(const glm::vec3& target);
    void SetDirection(const glm::vec3& direction);
    void Update();

private:
    glm::vec3 m_TargetPosition;
    glm::vec3 m_Direction;
    float m_DistanceBehind = 5.0f;
    float m_HeightOffset = 2.0f;
};
