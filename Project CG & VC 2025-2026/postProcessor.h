#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "shader.h"

class PostProcessor {
public:
    PostProcessor(int width, int height);
    ~PostProcessor();

    void beginScene();
    void endScene();
    void render();
    void renderBloom();

    void setEffect(int effect) { currentEffect = effect; }
    int getCurrentEffect() const { return currentEffect; }

    // Effect types
    enum Effect {
        NONE = 0,
        GAUSSIAN_BLUR = 1,
        EDGE_DETECTION = 2,
        BLOOM = 3
    };

    void setBloomThreshold(float threshold) { bloomThreshold = threshold; }
    void setBloomIntensity(float intensity) { bloomIntensity = intensity; }
    void setBloomPasses(int passes) { blurPasses = passes; }

    float getBloomThreshold() const { return bloomThreshold; }
    float getBloomIntensity() const { return bloomIntensity; }
    int getBloomPasses() const { return blurPasses; }

private:
    void setupFramebuffer();
    void setupQuad();
    void setupBloomFramebuffers();
    void cleanup();
    void renderQuad();

    unsigned int FBO;
    unsigned int colorTexture;
    unsigned int depthBuffer;
    unsigned int quadVAO, quadVBO;

    std::unique_ptr<Shader> postProcessShader;
    std::unique_ptr<Shader> brightPassShader;
    std::unique_ptr<Shader> blurShader;
    std::unique_ptr<Shader> bloomCombineShader;

    int screenWidth, screenHeight;
    int currentEffect;

    // Screen-aligned quad vertices
    float quadVertices[24] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int brightFBO;
    unsigned int brightTexture;
    unsigned int blurFBO[2];
    unsigned int blurTextures[2];

    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.8f;
    int blurPasses = 10;
};