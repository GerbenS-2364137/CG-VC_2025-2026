#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Bezier.h"
#include "FreeCam.h"
#include "FollowCam.h"
#include "CameraController.h"
#include "ChromaKeyOverlay.h"
#include "object.h"
#include "shader.h"
#include <ranges>


CameraController cameraController;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "Framebuffer resized to: " << width << "x" << height << std::endl;
}

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
    std::cout << "Starting OpenGL application..." << std::endl;

    // GLFW initialisatie
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    const int screenWidth = 800;
    const int screenHeight = 600;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL venster", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Kan geen venster maken!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD loading
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD faalt bij laden!\n";
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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

    // LUTs per curve
    std::vector<std::vector<Bezier::LookupEntry>> curveLUTs(curves.size());
    std::vector<float> curveLengths(curves.size());
    float totalDistance = 0.0f;

    for (size_t i = 0; i < curves.size(); ++i) {
        curveLUTs[i] = Bezier::BezierCurve::generateLookupTable(curves[i], 200);
        curveLengths[i] = curveLUTs[i].back().distance;
        totalDistance += curveLengths[i];
    }

    float animationSpeed = 3.0f;

    // Camera setup
    ChromaKeyOverlay overlay("textures/chromaKeyScreen.png");

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

        // Bepaal de positie van de kubus op de gecombineerde curve
        float d = fmod(currentFrame * animationSpeed, totalDistance);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        overlay.render();

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


        // Vind de juiste curve en lokale afstand
        float accumulated = 0.0f;
        int curveIndex = 0;
        for (size_t i = 0; i < curveLengths.size(); ++i) {
            if (d < accumulated + curveLengths[i]) {
                curveIndex = i;
                break;
            }
            accumulated += curveLengths[i];
        }

        float localDistance = d - accumulated;
        float t = Bezier::BezierCurve::findTforDistance(curveLUTs[curveIndex], localDistance);
        glm::vec3 curvePosition = curves[curveIndex].evaluate(t);

        renderWithLights(shader, vehicle, view, projection, model);


        // Bereken tangent (richting van beweging)
        float nextT = glm::clamp(t + 0.001f, 0.0f, 1.0f);
        glm::vec3 nextPos = curves[curveIndex].evaluate(nextT);
        glm::vec3 tangent = glm::normalize(nextPos - curvePosition);

        // Kies stabiele up-vector
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(tangent, up)) > 0.95f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f); // fallback als tangent bijna verticaal is
        }

        // Construct orthonormaal frame
        glm::vec3 right = glm::normalize(glm::cross(up, tangent));
        glm::vec3 correctedUp = glm::cross(tangent, right);

        // ROAD POSITIONING ADJUSTMENTS
        float roadHeightOffset = 0.25f;
        float pathOffset = 0.0f;
        float lateralOffset = 0.5f;

        // Calculate the final vehicle position
        glm::vec3 position = curvePosition + correctedUp * roadHeightOffset + tangent * pathOffset + right * lateralOffset;

        cameraController.updateFollowCamera(position, tangent); // <- wanneer object berekend wordt

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}