#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Bezier.h"
#include "FreeCam.h"
#include "FollowCam.h"
#include "CameraController.h"
#include "object.h"
#include "shader.h"


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

void renderWithLights(Shader& shader, Object& vehicle, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model) {
    // Make sure shader is active
    shader.use();

    // Check if shader is valid before setting uniforms
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program == 0) {
        std::cerr << "No shader program is currently active!" << std::endl;
        return;
    }

    // Set uniforms with error checking
    GLenum error = glGetError(); // Clear any previous errors

    shader.setVec3("viewPos", cameraController.getPosition());
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting viewPos uniform: " << error << std::endl;
    }

    // Send light data to shader
    // lightManager.sendToShader(shader);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error sending light data to shader: " << error << std::endl;
    }

    shader.setMat4("view", view);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting view matrix: " << error << std::endl;
    }

    shader.setMat4("projection", projection);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting projection matrix: " << error << std::endl;
    }

    shader.setMat4("model", model);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting model matrix: " << error << std::endl;
    }

    shader.setBool("useTexture", vehicle.usesTexture());
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error setting useTexture uniform: " << error << std::endl;
    }

    // Render vehicle
    vehicle.render();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error during vehicle.render(): " << error << std::endl;
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init mislukt\n";
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

    std::cout << "Loading shaders..." << std::endl;
    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");

    // Check if shader loaded successfully
    if (!shader.isValid()) {
        std::cerr << "Failed to load or compile shaders!" << std::endl;
        std::cerr << "Shader info log: " << shader.getInfoLog() << std::endl;
        return -1;
    }
    std::cout << "Shaders loaded successfully" << std::endl;

    std::cout << "Loading vehicle..." << std::endl;
    Object vehicle("objects/TeslaTruck.obj", "textures/cybertruck.jpeg");

    if (!vehicle.init()) {
        std::cerr << "Failed to initialize vehicle!" << std::endl;
        return -1;
    }
    std::cout << "Vehicle loaded successfully" << std::endl;

    std::cout << "Setting up Bezier curves..." << std::endl;
    int numCurves = 6;
    float radius = 10.0f;
    float hoogteFactor = 2.0f;

    // B�zier setup
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

        glm::vec3 modelCenter(-0.0349514, 0.019853, 0);
        float scaleFactor = 0.5f;

        // Build transformation matrix step by step for debugging
        glm::mat4 centerTransform = glm::translate(glm::mat4(1.0f), -modelCenter);
        glm::mat4 scaleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));
        glm::mat4 fixRotation = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        /*
        glm::mat4 rotationTransform = rotation;
        glm::mat4 positionTransform = glm::translate(glm::mat4(1.0f), position);
        */
        glm::mat4 model =  fixRotation * scaleTransform * centerTransform; // * positionTransform * rotationTransform

        glm::mat4 projection = glm::perspective(glm::radians(cameraController.getZoom()), 800.0f / 600.0f, 0.1f, 500.0f);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        shader.use();
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after shader.use(): " << error << std::endl;
        }

        // Check if shader compilation/linking was successful
        if (!shader.isValid()) {
            std::cerr << "Shader is not valid!" << std::endl;
            // Try to continue without breaking - maybe print shader info log
            std::cerr << "Shader info log: " << shader.getInfoLog() << std::endl;
        }

        renderWithLights(shader, vehicle, view, projection, model);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}