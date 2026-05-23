#include "CameraController.h"
#include <iostream>

CameraController::CameraController() {
    camera = std::make_unique<FreeCamera>();
}

void CameraController::switchToFollow() {
    camera = std::make_unique<FollowCamera>();
    followView = true;
    std::cout << "Switching to Follow camera mode" << std::endl;
}

void CameraController::switchToFree() {
    camera = std::make_unique<FreeCamera>(camera->Position, camera->Up, camera->Yaw, camera->Pitch, camera->Zoom);
    followView = false;
    std::cout << "Switching to Free camera mode" << std::endl;
}

void CameraController::processInput(GLFWwindow* window, float deltaTime) {
    // Toggle Free/Follow op C
    bool cPressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cPressed && !cWasPressed)
        followView ? switchToFree() : switchToFollow();
    cWasPressed = cPressed;

    // Bewegingstoetsen enkel actief in freecam
    if (!followView) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera->ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    }
}

void CameraController::processMouseMovement(float xoffset, float yoffset) {
    if (!followView)
        camera->ProcessMouseMovement(xoffset, yoffset);
}

void CameraController::processMouseScroll(float yoffset) {
    camera->ProcessMouseScroll(yoffset);
}

void CameraController::updateFollowCamera(const glm::vec3& targetPos, const glm::vec3& direction) {
    if (!followView) return;
    auto* follow = dynamic_cast<FollowCamera*>(camera.get());
    if (follow) {
        follow->SetTarget(targetPos);
        follow->SetDirection(direction);
        follow->Update();
    }
}

glm::mat4 CameraController::getViewMatrix() const {
    return camera->GetViewMatrix();
}

glm::vec3 CameraController::getPosition() const {
    return camera->Position;
}

float CameraController::getZoom() const {
    return camera->Zoom;
}