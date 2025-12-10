#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <memory>
#include <map>

#include "game/MainScene.h"
#include "shader.h"

//const GLuint WIDTH = 500;
//const GLuint HEIGHT = 750;
const GLuint WIDTH = 800;
const GLuint HEIGHT = 800;

GLuint shaderProgram;
GLint projLoc;
GLint viewLoc;
GLint modelLoc;
GLint colorLoc;
GLint texture1Loc;
GLint texture2Loc;
GLint cameraLoc;
GLint pointLightLoc;
GLint clipPlaneLoc;

GLFWwindow* window = nullptr;

std::map<unsigned char, bool> keyState;
std::map<int, bool> SpecialkeyState;
std::map<unsigned char, bool> prevkeyState;
std::map<int, bool> prevSpecialkeyState;
double xpos = 200.0; 
double ypos = 200.0;
bool firstMouse = true;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 256) {
        if (action == GLFW_PRESS) {
            keyState[static_cast<unsigned char>(key)] = true;
        } else if (action == GLFW_RELEASE) {
            keyState[static_cast<unsigned char>(key)] = false;
        }
    }
    // Handle special keys
    if (action == GLFW_PRESS) {
        SpecialkeyState[key] = true;
    } else if (action == GLFW_RELEASE) {
        SpecialkeyState[key] = false;
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint) {
    if (codepoint < 256) {
        // Handle character input for lowercase/uppercase
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {
    xpos = x;
    ypos = y;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::shared_ptr<MainScene> mainScene;

bool is_started = false;

void idle() {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { //ESC누렸을 떄 게임을 종료 (ESC is ASCII 27)
        std::cout << "Exiting the game." << std::endl;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } 

    if(!is_started && (keyState['z'] || keyState['Z'])) {
        //Z 버튼으로 게임을 시작
        mainScene = std::make_shared<MainScene>(); 
        std::cout << "Start the game." << std::endl; 
        is_started = true; 
    }
    
    if(is_started) { 
        if(mainScene) mainScene->update();
    }
    
    prevkeyState = keyState;
    prevSpecialkeyState = SpecialkeyState;
}

void display() {
    if(mainScene) mainScene->render(); 
}

int main(int argc, char** argv) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW 초기화 실패" << std::endl;
        return -1;
    }

    // Configure GLFW

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window
    window = glfwCreateWindow(WIDTH, HEIGHT, "< y >", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW 윈도우 생성 실패" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set GLFW callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
    // Set initial viewport to actual framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    
    // Setup cursor for FPS camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    shaderProgram = make_shader(&vertexShaderSource, &fragmentShaderSource);
    //shaderProgram = make_shader(&vertexShaderSource2, &fragmentShaderSource2);
    glUseProgram(shaderProgram);

    projLoc = glGetUniformLocation(shaderProgram, "projection");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    colorLoc = glGetUniformLocation(shaderProgram, "color");
    texture1Loc = glGetUniformLocation(shaderProgram, "texture1");
    texture2Loc = glGetUniformLocation(shaderProgram, "texture2");
    cameraLoc = glGetUniformLocation(shaderProgram, "cameraPos");
    pointLightLoc = glGetUniformLocation(shaderProgram, "pointLightPos");
    clipPlaneLoc = glGetUniformLocation(shaderProgram, "clipPlane");

    std::cout << "Press Z to start the game." << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        idle();
        display();
        
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}