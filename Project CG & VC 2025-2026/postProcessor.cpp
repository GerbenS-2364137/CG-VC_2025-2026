#include "postProcessor.h"
#include <iostream>

PostProcessor::PostProcessor(int width, int height)
    : screenWidth(width), screenHeight(height), currentEffect(NONE),
    FBO(0), colorTexture(0), depthBuffer(0), quadVAO(0), quadVBO(0),
    brightFBO(0), brightTexture(0) {

    std::cout << "Loading post-processing shaders..." << std::endl;

    // Load all shaders with error checking
    postProcessShader = std::make_unique<Shader>("post_process_vertex.glsl", "post_process_fragment.glsl");
    if (!postProcessShader->isValid()) {
        std::cerr << "Failed to load post-process shader!" << std::endl;
        std::cerr << postProcessShader->getInfoLog() << std::endl;
    }

    brightPassShader = std::make_unique<Shader>("post_process_vertex.glsl", "brightness_fragment.glsl");
    if (!brightPassShader->isValid()) {
        std::cerr << "Failed to load bright pass shader!" << std::endl;
        std::cerr << brightPassShader->getInfoLog() << std::endl;
    }

    blurShader = std::make_unique<Shader>("post_process_vertex.glsl", "blur_fragment.glsl");
    if (!blurShader->isValid()) {
        std::cerr << "Failed to load blur shader!" << std::endl;
        std::cerr << blurShader->getInfoLog() << std::endl;
    }

    bloomCombineShader = std::make_unique<Shader>("post_process_vertex.glsl", "bloom_combiner_fragment.glsl");
    if (!bloomCombineShader->isValid()) {
        std::cerr << "Failed to load bloom combine shader!" << std::endl;
        std::cerr << bloomCombineShader->getInfoLog() << std::endl;
    }

    std::cout << "Post-processing shaders loaded successfully!" << std::endl;

    setupFramebuffer();
    setupBloomFramebuffers();
    setupQuad();

    // Initialize blur framebuffers
    blurFBO[0] = blurFBO[1] = 0;
    blurTextures[0] = blurTextures[1] = 0;
}

PostProcessor::~PostProcessor() {
    cleanup();
}

void PostProcessor::setupFramebuffer() {
    // Generate main framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Create color texture
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Main framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::setupBloomFramebuffers() {
    // Bright pass framebuffer
    glGenFramebuffers(1, &brightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);

    glGenTextures(1, &brightTexture);
    glBindTexture(GL_TEXTURE_2D, brightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Bright pass framebuffer not complete!" << std::endl;
    }

    // Blur framebuffers (ping-pong)
    glGenFramebuffers(2, blurFBO);
    glGenTextures(2, blurTextures);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
        glBindTexture(GL_TEXTURE_2D, blurTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTextures[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: Blur framebuffer " << i << " not complete!" << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::setupQuad() {
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void PostProcessor::beginScene() {
    // Bind our framebuffer and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, screenWidth, screenHeight);

    // Clear the framebuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endScene() {
    // Bind back to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);

    render();
}

void PostProcessor::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessor::render() {
    if (currentEffect == BLOOM) {
        renderBloom();
        return;
    }
    // Disable depth testing for post-processing
    glDisable(GL_DEPTH_TEST);

    // Clear screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use post-processing shader
    postProcessShader->use();
    postProcessShader->setInt("screenTexture", 0);
    postProcessShader->setInt("effect", currentEffect);
    postProcessShader->setVec3("texelSize", 1.0f / (float)screenWidth, 1.0f / (float)screenHeight, 0.0f);

    // Bind the color texture from our framebuffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    renderQuad();

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}


void PostProcessor::cleanup() {
    if (FBO) {
        glDeleteFramebuffers(1, &FBO);
        FBO = 0;
    }
    if (colorTexture) {
        glDeleteTextures(1, &colorTexture);
        colorTexture = 0;
    }
    if (depthBuffer) {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
    if (blurFBO[0] || blurFBO[1]) {
        glDeleteFramebuffers(2, blurFBO);
        blurFBO[0] = blurFBO[1] = 0;
    }
    if (blurTextures[0] || blurTextures[1]) {
        glDeleteTextures(2, blurTextures);
        blurTextures[0] = blurTextures[1] = 0;
    }
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}

void PostProcessor::renderBloom() {
    glDisable(GL_DEPTH_TEST);

    // Take the bright parts
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    brightPassShader->use();
    brightPassShader->setInt("screenTexture", 0);
    brightPassShader->setFloat("threshold", bloomThreshold);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    renderQuad();

    // Apply gaussian blur on it
    blurShader->use();
    blurShader->setInt("screenTexture", 0);
    blurShader->setVec3("texelSize", 1.0f / (float)screenWidth, 1.0f / (float)screenHeight, 0.0f);

    bool horizontal = true;
    bool first_iteration = true;

    for (int i = 0; i < blurPasses; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
        blurShader->setBool("horizontal", horizontal);

        glActiveTexture(GL_TEXTURE0);
        // First iteration uses bright pass texture, subsequent use ping-pong
        glBindTexture(GL_TEXTURE_2D, first_iteration ? brightTexture : blurTextures[!horizontal]);

        renderQuad();
        horizontal = !horizontal;
        if (first_iteration) first_iteration = false;
    }

    // Combine it with the original frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    bloomCombineShader->use();
    bloomCombineShader->setInt("scene", 0);
    bloomCombineShader->setInt("bloomBlur", 1);
    bloomCombineShader->setFloat("bloomIntensity", bloomIntensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurTextures[!horizontal]);

    renderQuad();

    glEnable(GL_DEPTH_TEST);
}