#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <memory>
#include <map>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "layout (location = 2) in vec3 aTangent;\n"
    "layout (location = 3) in vec3 aBitangent;\n"
    "layout (location = 4) in vec2 aTexCoord;\n"
    "uniform vec4 clipPlane;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"
    "out vec2 TexCoord;\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec3 Tangent;\n"
    "out vec3 Bitangent;\n"
    "void main()\n"
    "{\n"
    "   FragPos = (model * vec4(aPos.x, aPos.y, aPos.z, 1.0)).xyz;\n"
    "   Normal = normalize(mat3(transpose(inverse(model))) * aNormal);\n"
    "   Tangent = normalize(mat3(transpose(inverse(model))) * aTangent);\n"
    "   Bitangent = normalize(mat3(transpose(inverse(model))) * aBitangent);\n"
    "   TexCoord = aTexCoord;\n"
    "   vec4 worldPos = model * vec4(aPos, 1.0);\n"
    "   gl_Position = projection * view * worldPos;\n"
    "   gl_ClipDistance[0] = dot(worldPos, clipPlane);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 FragPos;\n"
    "in vec3 Normal;\n"
    "in vec2 TexCoord;\n"
    "uniform vec3 cameraPos;\n"
    "uniform vec3 pointLightPos;\n"
    "uniform vec4 color;\n"
    "uniform sampler2D texture1;\n"
    "uniform sampler2D texture2;\n"
    "void main()\n"
    "{\n"
    "   vec3 lightDir = normalize(pointLightPos-FragPos);\n"
    "   vec3 cameraDir = normalize(cameraPos-FragPos);\n"
    "   vec3 h = normalize(lightDir+cameraDir);\n"
    "   vec3 n = normalize(Normal);\n"
    "   float l = max(dot(n, h), 0.1);\n"
    "   l = dot(n, normalize(2*n+texture(texture2, TexCoord).rgb));\n"
    "   FragColor = l*mix(texture(texture2, TexCoord), texture(texture1, TexCoord), 1.0);\n"
    "}\n\0";

GLuint make_shader(const GLchar **vertexSourceString, const GLchar **fragmentSourceString) {
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, vertexSourceString, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, fragmentSourceString, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}