#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Bezier.h"
#include "FreeCam.h"
#include "FollowCam.h"
#include "CameraController.h"

CameraController cameraController;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed
    lastX = (float)xpos;
    lastY = (float)ypos;

    cameraController.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraController.processMouseScroll((float)yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    cameraController.processInput(window, deltaTime);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    const int screenWidth = 800;
    const int screenHeight = 600;

    std::cout << "OpenGL versie: " << glGetString(GL_VERSION) << "\n";
    glEnable(GL_DEPTH_TEST);


    std::cout << "Setting up Bezier curves..." << std::endl;
    int numCurves = 6;
    float radius = 10.0f;
    float hoogteFactor = 2.0f;

    // Bézier setup
    std::vector<Bezier::BezierCurve> curves = Bezier::generateClosedBezierPath(numCurves, radius, hoogteFactor);

    // Camera setup
    

    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {
        frameCount++;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // cameraController.updateFollowCamera(objectPos, tangent, deltaTime); // <- wanneer object berekend wordt
        glm::mat4 view = cameraController.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(cameraController.getZoom()), 800.0f / 600.0f, 0.1f, 500.0f);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}