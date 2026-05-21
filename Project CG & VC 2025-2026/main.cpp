#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Bezier.h"

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

    std::vector<Bezier::BezierCurve> curves = Bezier::generateClosedBezierPath(numCurves, radius, hoogteFactor);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}