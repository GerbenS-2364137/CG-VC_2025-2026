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
#include "BezierVisualiser.h"
#include "lichtbron.h"
#include "postProcessor.h"
#include <ranges>

CameraController cameraController;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

LightManager lightManager;
std::vector<glm::vec3> colors = {
    glm::vec3(1.0f, 0.0f, 0.0f), // Rood
    glm::vec3(0.0f, 1.0f, 0.0f), // Groen
    glm::vec3(0.0f, 0.0f, 1.0f), // Blauw
    glm::vec3(1.0f, 1.0f, 1.0f), // Wit
    glm::vec3(1.0f, 1.0f, 0.0f), // Geel
    glm::vec3(1.0f, 0.0f, 1.0f), // Magenta
    glm::vec3(0.0f, 1.0f, 1.0f)  // Cyaan
};
int currentColorIndex = 0;

std::unique_ptr<PostProcessor> postProcessor;

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

void processLightChange(GLFWwindow* window) {
    // Wissel kleur met L - toets
    static bool lWasPressed = false;
    bool lPressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    if (lPressed && !lWasPressed) {
        currentColorIndex = (currentColorIndex + 1) % colors.size();
        for (int i = 0; i < lightManager.getLightCount(); ++i) {
            lichtbron& light = lightManager.getLight(i);
            light.color = colors[currentColorIndex];

            lightManager.updateLight(i, light);
        }
    }
    lWasPressed = lPressed;
}

void processPostProcess(GLFWwindow* window) {
    static bool oneWasPressed = false;
    static bool twoWasPressed = false;
    static bool threeWasPressed = false;
    static bool fourWasPressed = false;

    bool onePressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    bool twoPressed = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    bool threePressed = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    bool fourPressed = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;

    if (onePressed && !oneWasPressed) {
        postProcessor->setEffect(PostProcessor::NONE);
        std::cout << "Post-processing: None" << std::endl;
    }
    if (twoPressed && !twoWasPressed) {
        postProcessor->setEffect(PostProcessor::GAUSSIAN_BLUR);
        std::cout << "Post-processing: Blur" << std::endl;
    }
    if (threePressed && !threeWasPressed) {
        postProcessor->setEffect(PostProcessor::EDGE_DETECTION);
        std::cout << "Post-processing: Edge Detection" << std::endl;
    }
    if (fourPressed && !fourWasPressed) {
        postProcessor->setEffect(PostProcessor::BLOOM);
        std::cout << "Post-processing: Bloom Effect" << std::endl;
    }

    oneWasPressed = onePressed;
    twoWasPressed = twoPressed;
    threeWasPressed = threePressed;
    fourWasPressed = fourPressed;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    cameraController.processInput(window, deltaTime);

    processLightChange(window);
    processPostProcess(window);
}



void setupLights() {
    lichtbron l1(
        glm::vec3(5.0f, 8.0f, 0.0f), //pos
        glm::vec3(1.0f, 1.0f, 1.0f),  //kleur
        3.0f,  //intensiteit
        1.0f,
        0.045f,
        0.0075f
    );
    lightManager.addLight(l1);
    std::cout << "Light setup complete" << std::endl;
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

    setupLights();

    std::cout << "Setting up Bezier curves..." << std::endl;
    int numCurves = 6;
    float radius = 10.0f;
    float hoogteFactor = 2.0f;

    // B�zier setup
    std::vector<Bezier::BezierCurve> curves = Bezier::generateClosedBezierPath(numCurves, radius, hoogteFactor);

    std::vector<std::unique_ptr<BezierVisualiser>> visualizers(curves.size());
    for (size_t i = 0; i < curves.size(); ++i) {
        visualizers[i] = std::make_unique<BezierVisualiser>();


        visualizers[i]->init(curves[i], "objects/roadV2.obj", "textures/road.jpg");

        // Optional: Configure the visualizer
        visualizers[i]->setObjectSpacing(0.0f);  // Spacing between objects
        visualizers[i]->setZSquishFactor(0.2f);  // How much to squish along Z-axis
        visualizers[i]->setObjectScale(0.35f);   // Overall scale of path objects
    }

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

    // initialize de postprocessor
    std::cout << "Initializing PostProcessor..." << std::endl;
    postProcessor = std::make_unique<PostProcessor>(screenWidth, screenHeight);

    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {

        frameCount++;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        postProcessor->beginScene();

        // Bepaal de positie van de auto op de gecombineerde curve
        float d = fmod(currentFrame * animationSpeed, totalDistance);

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

        // Bereken tangent (richting van beweging)
        glm::vec3 tangent = curves[curveIndex].tangent(t);

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

        cameraController.updateFollowCamera(position, tangent);

        // Bouw rotatie matrix uit right-up-forward (TBN-frame)
        glm::mat4 rotation(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(correctedUp, 0.0f);
        rotation[2] = glm::vec4(tangent, 0.0f);



        glm::vec3 modelCenter(-0.0349514, 0.019853, 0);
        float scaleFactor = 0.5f;

        // Build transformation matrix step by step for debugging
        glm::mat4 centerTransform = glm::translate(glm::mat4(1.0f), -modelCenter);
        glm::mat4 scaleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));
        glm::mat4 fixRotation = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 rotationTransform = rotation;
        glm::mat4 positionTransform = glm::translate(glm::mat4(1.0f), position);

        glm::mat4 model =  positionTransform * rotationTransform * fixRotation * scaleTransform * centerTransform;

        glm::mat4 view = cameraController.getViewMatrix();

        glm::mat4 projection = glm::perspective(glm::radians(cameraController.getZoom()), 800.0f / 600.0f, 0.1f, 500.0f);

        glEnable(GL_DEPTH_TEST);
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setInt("texture1", 0);

        vehicle.renderWithLights(shader, lightManager, cameraController, view, projection, model);

        for (size_t i = 0; i < curves.size(); ++i) {
            visualizers[i]->render(view, projection, lightManager, cameraController.getPosition());
        }

        overlay.render();
        postProcessor->endScene();

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

        if (!postProcessor) {
            std::cerr << "PostProcessor is null!" << std::endl;
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}