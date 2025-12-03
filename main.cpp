#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <memory>
#include <map>

#include "game/MainScene.h"
#include "shader.h"

const GLuint WIDTH = 500;
const GLuint HEIGHT = 750;

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

std::map<unsigned char, bool> keyState;
std::map<int, bool> SpecialkeyState;
std::map<unsigned char, bool> prevkeyState;
std::map<int, bool> prevSpecialkeyState;
int xpos; int ypos;

void handleKeyDown(unsigned char key, int x, int y) { keyState[key] = true; }
void handleKeyUp(unsigned char key, int x, int y) { keyState[key] = false; }
void handleSpecialKeyDown(int key, int x, int y) { SpecialkeyState[key] = true; }
void handleSpecialKeyUp(int key, int x, int y) { SpecialkeyState[key] = false; }
void handleMouseMove(int x, int y) {xpos = x; ypos = y; glutWarpPointer(200, 200); }

std::shared_ptr<MainScene> mainScene;

bool is_started = false;

void idle() {
    if(keyState[27]) { //ESC누렸을 떄 게임을 종료
        std::cout << "Exiting the game." << std::endl; glutLeaveMainLoop();  
    } 

    if(!is_started && (keyState['z'] || keyState['Z'])) {
        //Z 버튼으로 게임을 시작
        mainScene = std::make_shared<MainScene>(); 
        std::cout << "Start the game." << std::endl; 
        is_started = true; 
    }
    
    if(is_started) { 
        if(mainScene) mainScene->update(); 
        glutPostRedisplay(); 
    }
    
    prevkeyState = keyState;
    prevSpecialkeyState = SpecialkeyState;
}

void display() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(mainScene) mainScene->render(); 
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("< y >");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

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
    
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutSpecialFunc(handleSpecialKeyDown);
    glutSpecialUpFunc(handleSpecialKeyUp);
    glutPassiveMotionFunc(handleMouseMove);

    glutIdleFunc(idle);
    glutDisplayFunc(display);

    std::cout << "Press Z to start the game." << std::endl;
    glutMainLoop();
    return 0;
}