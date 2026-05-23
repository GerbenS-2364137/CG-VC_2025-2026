#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // Initialize to invalid state
    ID = 0;
    compilationSuccessful = false;
    linkingSuccessful = false;

    // Laad de shaderbron uit bestanden
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile(vertexPath);
    std::ifstream fShaderFile(fragmentPath);

    // Check if files opened successfully
    if (!vShaderFile.is_open()) {
        std::cerr << "ERROR: Could not open vertex shader file: " << vertexPath << std::endl;
        lastError = "Could not open vertex shader file: " + std::string(vertexPath);
        return;
    }
    if (!fShaderFile.is_open()) {
        std::cerr << "ERROR: Could not open fragment shader file: " << fragmentPath << std::endl;
        lastError = "Could not open fragment shader file: " + std::string(fragmentPath);
        return;
    }

    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    std::cout << "Vertex shader loaded (" << vertexCode.length() << " characters)" << std::endl;
    std::cout << "Fragment shader loaded (" << fragmentCode.length() << " characters)" << std::endl;

    // Compileer de vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        lastError = "Vertex shader compilation failed: " + std::string(infoLog);
        glDeleteShader(vertex);
        return;
    }
    else {
        std::cout << "Vertex shader compiled successfully!" << std::endl;
    }

    // Compileer de fragment shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        lastError = "Fragment shader compilation failed: " + std::string(infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }
    else {
        std::cout << "Fragment shader compiled successfully!" << std::endl;
    }

    compilationSuccessful = true;

    // Link shaders naar shaderprogramma
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        lastError = "Shader program linking failed: " + std::string(infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(ID);
        ID = 0;
        return;
    }
    else {
        std::cout << "Shader program linked successfully!" << std::endl;
        linkingSuccessful = true;
    }

    // Verwijder de individuele shaders na het linken
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Clear any previous error message since everything succeeded
    lastError = "";
}

void Shader::use() {
    if (isValid()) {
        glUseProgram(ID);
    }
    else {
        std::cerr << "Warning: Trying to use invalid shader program!" << std::endl;
    }
}

bool Shader::isValid() const {
    return (ID != 0 && compilationSuccessful && linkingSuccessful);
}

std::string Shader::getInfoLog() const {
    return lastError;
}

void Shader::setBool(const std::string& name, bool value) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1i(location, (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1f(location, value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform3fv(location, 1, &value[0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    if (!isValid()) {
        std::cerr << "Warning: Setting uniform on invalid shader: " << name << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform3f(location, x, y, z);
}