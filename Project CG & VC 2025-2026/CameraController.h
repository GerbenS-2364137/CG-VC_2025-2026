#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include "Camera.h"
#include "FreeCam.h"
#include "FollowCam.h"

class CameraController
{
public:
	CameraController();

	glm::mat4 getViewMatrix()  const;
	glm::vec3 getPosition()    const;

	float getZoom()        const;

	bool isFollowMode()   const { return followView; }

	// Verwerkt toetsenbord en camera toggle. Elke frame aanroepen
	void processInput(GLFWwindow* window, float deltaTime);

	// Verwerk muisbeweging
	void processMouseMovement(float xoffset, float yoffset);

	// Update follow-camera naar positie/richting van het object
	void updateFollowCamera(const glm::vec3& targetPos, const glm::vec3& direction);

private:
	std::unique_ptr<Camera> camera;
	bool followView = false;
	bool cWasPressed = false;

	void switchToFollow();
	void switchToFree();
};

