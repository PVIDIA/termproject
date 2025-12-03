#include "MainScene.h"

#include <map>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <stack>

std::random_device rd; 
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dist(1, 4);
std::normal_distribution<float> distf(-1.0f, 1.0f);

extern std::map<unsigned char, bool> keyState;
extern std::map<int, bool> SpecialkeyState;
extern std::map<unsigned char, bool> prevkeyState;
extern std::map<int, bool> prevSpecialkeyState;
extern int xpos; 
extern int ypos;
extern int render_mode;
extern bool render_collision;

extern glm::mat4 model_matrix;
extern std::stack<glm::mat4> stack_model_matrix;

extern GLint pointLightLoc; //임시
extern GLint clipPlaneLoc;

std::map<std::string, int> timerLock;
//0이면 사용할 수 있음, idle마다 1씩 감소

// __________ MainScene __________

MainScene::MainScene() : SceneNode("MainScene"), PortalTest() {

    std::shared_ptr<Node> node;

    //__________ Camera ___________
    cameras.push_back(std::make_shared<PersCameraNode>("Camera1", glm::vec3(-300.0, 200.0f, -300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f));
    cameras.push_back(portalTest.p1_camera);
    cameras.push_back(portalTest.p2_camera);
    addChild(cameras[0]); //Camera push : WorldPersCamera(0)
    current_camera_num = 0;
    main_camera = cameras[current_camera_num];

    node = std::make_shared<Test>();
    addChild(node); node = nullptr;

    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0), glm::angleAxis(float(glm::radians(0.0)), glm::vec3(0.0, 1.0, 0.0)), 500.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_white", "normal_industrial"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(500.0, 500.0, 0.0), glm::angleAxis(float(glm::radians(90.0)), glm::vec3(0.0, 0.0, 1.0)), 500.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_white", "normal_industrial"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(-500.0, 500.0, 0.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(0.0, 0.0, 1.0)), 500.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_white", "normal_industrial"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0, 500.0, -500.0), glm::angleAxis(float(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0)), 500.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_white", "normal_industrial"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0, 500.0, 500.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(1.0, 0.0, 0.0)), 500.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_white", "normal_industrial"));
    
    addChild(std::make_shared<ModelNode>("portal", glm::vec3(200.0, 1.0, 0.0), glm::angleAxis(0.0, glm::vec3(1.0, 0.0, 0.0)), 100.0, glm::vec3(0.0, 1.0, 0.0), "", ""));
    addChild(std::make_shared<ModelNode>("portal", glm::vec3(-200.0, 1.0, 0.0), glm::angleAxis(0.0, glm::vec3(1.0, 0.0, 0.0)), 100.0, glm::vec3(0.0, 1.0, 0.0), "", ""));

    std::cout << "-- portal1 --" << std::endl;
    collectCollisions();
}
void MainScene::render() {
    auto temp = main_camera;
    glBindFramebuffer(GL_FRAMEBUFFER, portalTest.p1_FBO);
    //glViewport(0, 0, texWidth, texHeight);
    main_camera = cameras[1];
    SceneNode::render();

    glBindFramebuffer(GL_FRAMEBUFFER, portalTest.p2_FBO);
    main_camera = cameras[2];
    SceneNode::render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    main_camera = temp;
    SceneNode::render();
}

float qwerasdf = 0.0f;
void MainScene::update() {
    //glUniform3f(pointLightLoc, qwerasdf, 500.0, qwerasdf); //임시
    glUniform3f(pointLightLoc, 0.0, qwerasdf+100.0, 0.0); //임시
    qwerasdf += 5.0f;
    qwerasdf = qwerasdf > 500.0 ? qwerasdf-500.0 : qwerasdf;
    
    portalTest.update(cameras[0]->position);
    
    glEnable(GL_CLIP_DISTANCE0);
    if(main_camera == cameras[0]) {
        glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 planePoint  = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec4 clipPlane(planeNormal.x, planeNormal.y, planeNormal.z,
                            -glm::dot(planeNormal, planePoint)  // D = -n·p
        );
        glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
    }
    else if(main_camera == cameras[2]) {
        // 평면: point = (500,0,0), normal = (-1,0,0)
        glm::vec3 planeNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
        glm::vec3 planePoint  = glm::vec3(499.0f, 0.0f, 0.0f);
        glm::vec4 clipPlane(planeNormal.x, planeNormal.y, planeNormal.z,
                            -glm::dot(planeNormal, planePoint)  // D = -n·p
        );
        glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
    }
    else if(main_camera == cameras[1]) {
        // 평면: point = (500,0,0), normal = (-1,0,0)
        glm::vec3 planeNormal = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 planePoint  = glm::vec3(0.0f, 0.0f, 499.0f);
        glm::vec4 clipPlane(planeNormal.x, planeNormal.y, planeNormal.z,
                            -glm::dot(planeNormal, planePoint)  // D = -n·p
        );
        glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
    }

    std::cout << "mouse position : (" << xpos-200 << ", " << ypos-200 << ")" << std::endl;
    for (auto& [key, value] : timerLock) if(value > 0) value--;
    time++;
    
    //Camera change
    if( (!prevkeyState['q'] && keyState['q']) || (!prevkeyState['Q'] && keyState['Q'])) {
        current_camera_num = (current_camera_num+1)%cameras.size();
        main_camera = cameras[current_camera_num];
        std::cout << "Camera changed : " << main_camera->name << std::endl;
    }

    //Render change
    if( (!prevkeyState['w'] && keyState['w']) || (!prevkeyState['W'] && keyState['W'])) {
        render_mode++;
        std::cout << "Render mode changed" << std::endl;
    }

    //Collision render option
    if( (!prevkeyState['e'] && keyState['e']) || (!prevkeyState['E'] && keyState['E'])) {
        render_collision = !render_collision;
        if(render_collision)
            std::cout << "Collision render activated" << std::endl;
        else
            std::cout << "Collision render deactivated" << std::endl;
    }

    //Main camera move
    if(keyState['i'] || keyState['I']) cameras[0]->position.x += 1.0;
    if(keyState['k'] || keyState['K']) cameras[0]->position.x -= 1.0;
    if(keyState['j'] || keyState['J']) cameras[0]->position.z -= 1.0;
    if(keyState['l'] || keyState['L']) cameras[0]->position.z += 1.0;
    if(main_camera == cameras[0]) {
        if(xpos != 200 || ypos != 200) {
            float delta_x = xpos - 200.0;
            float delta_y = ypos - 200.0;
            
            float sensitivity = 0.002f;
            glm::vec3 right = glm::normalize(glm::cross(main_camera->direction, main_camera->up));
            glm::quat qYaw = glm::angleAxis(-delta_x * sensitivity, main_camera->up);
            glm::quat qPitch = glm::angleAxis(-delta_y * sensitivity, right);
            glm::quat q = qYaw * qPitch;

            main_camera->direction = glm::normalize(q * main_camera->direction);
        }
    }
    /*
    if(main_camera == cameras[0]) {
        if(keyState['u'] || keyState['U']) main_camera->position += 120.0f*main_camera->direction/60.0f;
        if(keyState['o'] || keyState['O']) main_camera->position -= 120.0f*main_camera->direction/60.0f;
        if(keyState['i'] || keyState['I']) main_camera->position += 120.0f*main_camera->up/60.0f;
        if(keyState['k'] || keyState['K']) main_camera->position -= 120.0f*main_camera->up/60.0f;
        if(keyState['j'] || keyState['J']) main_camera->position -= 120.0f*glm::cross(main_camera->direction, main_camera->up)/60.0f;
        if(keyState['l'] || keyState['L']) main_camera->position += 120.0f*glm::cross(main_camera->direction, main_camera->up)/60.0f;
        if(xpos != 200 || ypos != 200) {
            float delta_x = xpos - 200.0;
            float delta_y = ypos - 200.0;
            
            float sensitivity = 0.002f;
            glm::vec3 right = glm::normalize(glm::cross(main_camera->direction, main_camera->up));
            glm::quat qYaw = glm::angleAxis(-delta_x * sensitivity, main_camera->up);
            glm::quat qPitch = glm::angleAxis(-delta_y * sensitivity, right);
            glm::quat q = qYaw * qPitch;

            main_camera->direction = glm::normalize(q * main_camera->direction);
        }
    }
    */
    SceneNode::update();
}


Test::Test() : ObjectNode("Test", glm::vec3(100.0, 100.0, 100.0), glm::vec3(0.0, 0.0, 1.0)) {
    addChild(std::make_shared<ModelNode>("rice", glm::vec3(0.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(0.0, 1.0, 0.0)), 50.0, glm::vec3(1.0, 1.0, 0.0), "diffuse_rice", "normal_cobble"));
}

void Test::update() {
    if(keyState['a'] || keyState['A']) rotation *= glm::angleAxis(0.03f, glm::vec3(1.0, 0.0, 0.0));
    if(keyState['s'] || keyState['S']) rotation *= glm::angleAxis(-0.03f, glm::vec3(1.0, 0.0, 0.0));
    if(keyState['z'] || keyState['Z']) rotation *= glm::angleAxis(0.03f, glm::vec3(0.0, 0.0, 1.0));
    if(keyState['x'] || keyState['X']) rotation *= glm::angleAxis(-0.03f, glm::vec3(0.0, 0.0, 1.0));

    ObjectNode::update();
}
void Test::render() {
    ObjectNode::render();
}

PortalTest::PortalTest() : ObjectNode("PortalTest", glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0)) {
    //portal
    portal1_position = glm::vec3(500, 200.0, 0.0);
    portal1_direction = glm::vec3(-1.0, 0.0, 0.0);
    portal1_up = glm::vec3(0.0, 1.0, 0.0);
    
    portal2_position = glm::vec3(0.0, 200.0, 500.0);
    portal2_direction = glm::vec3(0.0, 0.0, -1.0);
    portal2_up = glm::vec3(0.0, 1.0, 0.0);

    p1_camera = std::make_shared<PersCameraNode>("p1", glm::vec3(300.0, 200.0f, 300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f);
    p2_camera = std::make_shared<PersCameraNode>("p2", glm::vec3(300.0, 200.0f, 300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f);

    glGenTextures(1, &p1_texture);
    glBindTexture(GL_TEXTURE_2D, p1_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenTextures(1, &p2_texture);
    glBindTexture(GL_TEXTURE_2D, p2_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &p1_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, p1_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p1_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenFramebuffers(1, &p2_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, p2_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p2_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PortalTest::update(const glm::vec3& viewer_position) {
    //protal
    //portal local world -> 
    glm::mat4 portal1World = glm::mat4(
                                        glm::vec4(glm::cross(portal1_up, portal1_direction), 0.0f),
                                        glm::vec4(portal1_up, 0.0f),
                                        glm::vec4(portal1_direction, 0.0f),
                                        glm::vec4(portal1_position, 1.0f)
                                    );
    //portal1World = glm::transpose(portal1World);
    glm::mat4 portal1World_inverse = glm::inverse(portal1World);
    //portal local world -> 
    glm::mat4 portal2World = glm::mat4(
                                        glm::vec4(glm::cross(portal2_up, portal2_direction), 0.0f),
                                        glm::vec4(portal2_up, 0.0f),
                                        glm::vec4(portal2_direction, 0.0f),
                                        glm::vec4(portal2_position, 1.0f)
                                    );
    //portal2World = glm::transpose(portal2World);
    glm::mat4 portal2World_inverse = glm::inverse(portal2World);
    glm::mat4 flip180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0,1,0)); //y축 180 회전

    p1_camera->position = glm::vec3(portal2World * flip180 * portal1World_inverse * glm::vec4(viewer_position, 1.0));
    p2_camera->position = glm::vec3(portal1World * flip180 * portal2World_inverse * glm::vec4(viewer_position, 1.0));

    p1_camera->direction = glm::normalize(portal2_position - p1_camera->position);
    p2_camera->direction = glm::normalize(portal1_position - p2_camera->position);

    p1_camera->up = glm::vec3(0.0, 1.0, 0.0);
    p2_camera->up = glm::vec3(0.0, 1.0, 0.0);

    std::cout << "-- portal1 --" << std::endl;
    std::cout << "position : (" << portal1_position.x << ", " << portal1_position.y << ", "  << portal1_position.z << ")" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "-- portal2 --" << std::endl;
    std::cout << "position : (" << portal2_position.x << ", " << portal2_position.y << ", "  << portal2_position.z << ")" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "-- p1_camera --" << std::endl;
    std::cout << "position : (" << p1_camera->position.x << ", " << p1_camera->position.y << ", "  << p1_camera->position.z << ")" << std::endl;
    std::cout << "direciton : (" << p1_camera->direction.x << ", " << p1_camera->direction.y << ", "  << p1_camera->direction.z << ")" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "-- p2_camera --" << std::endl;
    std::cout << "position : (" << p2_camera->position.x << ", " << p2_camera->position.y << ", "  << p2_camera->position.z << ")" << std::endl;
    std::cout << "direciton : (" << p2_camera->direction.x << ", " << p2_camera->direction.y << ", "  << p2_camera->direction.z << ")" << std::endl;
    std::cout << "-------------" << std::endl;
}
void PortalTest::render() {

}