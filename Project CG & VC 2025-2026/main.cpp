#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Bezier.h"
#include "FreeCam.h"
#include "FollowCam.h"

// Camera
std::unique_ptr<Camera> camera;
bool followView = false;  // Voor toggling tussen Free en Follow
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

void processCameraType(GLFWwindow* window) {
    static bool cWasPressed = false;  // Houdt de vorige status van de toets bij
    bool cPressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

    // De status omdraaien bij het loslaten van de toets
    if (cPressed && !cWasPressed) {
        followView = !followView;  // Wissel tussen views
        std::cout << "Switching to " << (followView ? "Follow" : "Free") << " camera mode" << std::endl;

        if (followView) {
            // Zorg ervoor dat FollowCamera correct is geërfd van Camera
            camera = std::make_unique<FollowCamera>();
        }
        else {
            // Zorg ervoor dat FreeCamera correct is geërfd van Camera
            camera = std::make_unique<FreeCamera>(camera->Position, camera->Up, camera->Yaw, camera->Pitch, camera->Zoom);
        }
    }

    // Bijhouden of de toets ingedrukt was voor de volgende frame
    cWasPressed = cPressed;

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

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Free-cam / Follow-cam
    processCameraType(window);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed
    lastX = xpos;
    lastY = ypos;

    if (!followView) {
        camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init mislukt\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Venster aanmaken mislukt\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init mislukt\n";
        return -1;
    }

    std::cout << "OpenGL versie: " << glGetString(GL_VERSION) << "\n";


    std::cout << "Setting up Bezier curves..." << std::endl;
    int numCurves = 6;
    float radius = 10.0f;
    float hoogteFactor = 2.0f;

    // Bézier setup
    std::vector<Bezier::BezierCurve> curves = Bezier::generateClosedBezierPath(numCurves, radius, hoogteFactor);

    // Camera setup
    camera = std::make_unique<FreeCamera>();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}