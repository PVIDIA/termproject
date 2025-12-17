#include "MainScene.h"

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/intersect.hpp>
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
extern double xpos; 
extern double ypos;
extern bool leftMousePressed;
extern bool rightMousePressed;
extern int render_mode;
extern bool render_collision;

extern glm::mat4 model_matrix;
extern std::stack<glm::mat4> stack_model_matrix;

extern GLint pointLightLoc; //임시
extern GLint clipPlaneLoc;

std::map<std::string, int> timerLock;
//0이면 사용할 수 있음, idle마다 1씩 감소

extern bool isRenderingPortal;


// __________ MainScene __________

MainScene::MainScene() : SceneNode("MainScene") {

    std::shared_ptr<Node> node;

    //__________ Camera ___________
    cameras.push_back(std::make_shared<PersCameraNode>("Camera1", glm::vec3(-300.0, 200.0f, -300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f, 1.0f));
    addChild(cameras[0]); //Camera push : WorldPersCamera(0)
    current_camera_num = 0;
    main_camera = cameras[current_camera_num];
    
    // Initialize previous camera position
    prevCameraPosition = cameras[0]->position;

    node = std::make_shared<Test>();
    addChild(node); node = nullptr;

    player = std::make_shared<Player>();
    addChild(player);

    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0), glm::angleAxis(float(glm::radians(0.0)), glm::vec3(0.0, 1.0, 0.0)), 500.0f, glm::vec3(1.0, 1.0, 0.0), "CONCRETECONCRETE_MODULAR_CEILING001A_baseColor", "normal_cobble"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(500.0, 500.0, 0.0), glm::angleAxis(float(glm::radians(90.0)), glm::vec3(0.0, 0.0, 1.0)), 500.0f, glm::vec3(1.0, 1.0, 0.0), "CONCRETECONCRETE_MODULAR_CEILING001A_baseColor", "normal_cobble"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(-500.0, 500.0, 0.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(0.0, 0.0, 1.0)), 500.0f, glm::vec3(1.0, 1.0, 0.0), "CONCRETECONCRETE_MODULAR_CEILING001A_baseColor", "normal_cobble"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0, 500.0, -500.0), glm::angleAxis(float(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0)), 500.0f, glm::vec3(1.0, 1.0, 0.0), "CONCRETECONCRETE_MODULAR_CEILING001A_baseColor", "normal_cobble"));
    addChild(std::make_shared<ModelNode>("repeating_plane", glm::vec3(0.0, 500.0, 500.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(1.0, 0.0, 0.0)), 500.0f, glm::vec3(1.0, 1.0, 0.0), "CONCRETECONCRETE_MODULAR_CEILING001A_baseColor", "normal_cobble"));
    
    std::cout << "-- portal1 --" << std::endl;
    collectCollisions();
}
void MainScene::render() {

    extern GLFWwindow* window;
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    //화면 지우기
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // world render 
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    isRenderingPortal = false;
    SceneNode::render();
    std::shared_ptr<CameraNode> temp = main_camera;


    glm::mat4 portal1World = portalManager.portal1.local_to_wolrd();
    glm::mat4 portal1World_inverse = glm::inverse(portal1World);
    glm::mat4 portal2World = portalManager.portal2.local_to_wolrd();
    glm::mat4 portal2World_inverse = glm::inverse(portal2World);
    glm::mat4 flip180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0,1,0));
    glm::mat4 m12 = portal2World * flip180 * portal1World_inverse;
    glm::mat4 m21 = portal1World * flip180 * portal2World_inverse;
/*
    // potal stencil 값 생성 및 깊이, 생각 초기화
    portalManager.draw_stencil();

    // portal 1 내부 그리기
    glEnable(GL_CLIP_DISTANCE0);

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    portalManager.portal1_camera->position = glm::vec3(m12 * glm::vec4(portalManager.portal1_camera->position, 1.0));
    portalManager.portal1_camera->direction = glm::mat3(m12) * portalManager.portal1_camera->direction;
    portalManager.portal1_camera->up = glm::mat3(m12) * portalManager.portal1_camera->up;
    main_camera = portalManager.portal1_camera;

    portalManager.setPortal1Clipping();

    isRenderingPortal = true;
    SceneNode::render();
    
    // portal 2 내부 그리기
    glStencilFunc(GL_EQUAL, 2, 0xFF);
    portalManager.portal2_camera->position = glm::vec3(m21 * glm::vec4(portalManager.portal2_camera->position, 1.0));
    portalManager.portal2_camera->direction = glm::mat3(m21) * portalManager.portal2_camera->direction;
    portalManager.portal2_camera->up = glm::mat3(m21) * portalManager.portal2_camera->up;
    main_camera = portalManager.portal2_camera;

    portalManager.setPortal2Clipping();

    isRenderingPortal = true;
    SceneNode::render();

    glDisable(GL_CLIP_DISTANCE0);
*/
    render_recursive(0, 0, glm::mat4(1.0f), glm::mat4(1.0f), m12, m21);

    main_camera = temp;
}

void MainScene::render_recursive(int depth, int max_depth, const glm::mat4& portal1_matrix, const glm::mat4& portal2_matrix, const glm::mat4& m12, const glm::mat4& m21) {
    if(depth > max_depth) return;
    //화면 지우기

    portalManager.draw_stencil(depth, portal1_matrix, portal2_matrix);

    // portal 1 내부 그리기
    glEnable(GL_CLIP_DISTANCE0);

    auto temp1 = portalManager.portal1_camera->position;
    auto temp2 = portalManager.portal1_camera->direction;
    auto temp3 = portalManager.portal1_camera->up;

    glStencilFunc(GL_EQUAL, depth+1, 0xFF);
    portalManager.portal1_camera->position = glm::vec3(m12 * glm::vec4(portalManager.portal2_camera->position, 1.0));
    portalManager.portal1_camera->direction = glm::mat3(m12) * portalManager.portal2_camera->direction;
    portalManager.portal1_camera->up = glm::mat3(m12) * portalManager.portal2_camera->up;
    main_camera = portalManager.portal1_camera;

    portalManager.setPortal1Clipping(portal1_matrix);

    isRenderingPortal = true;
    SceneNode::render();
    
    // portal 2 내부 그리기
    glStencilFunc(GL_EQUAL, 128+depth+1, 0xFF);
    portalManager.portal2_camera->position = glm::vec3(m21 * glm::vec4(temp1, 1.0));
    portalManager.portal2_camera->direction = glm::mat3(m21) * temp2;
    portalManager.portal2_camera->up = glm::mat3(m21) * temp3;
    main_camera = portalManager.portal2_camera;

    portalManager.setPortal2Clipping(portal2_matrix);

    SceneNode::render();

    glDisable(GL_CLIP_DISTANCE0);

    render_recursive(depth+1, max_depth, portal1_matrix*m12, portal2_matrix*m21, m21, m12);
}

float qwerasdf = 0.0f;
void MainScene::update() {
    //glUniform3f(pointLightLoc, qwerasdf, 500.0, qwerasdf); //임시
    glUniform3f(pointLightLoc, 0.0, qwerasdf+100.0, 0.0); //임시
    qwerasdf += 5.0f;
    qwerasdf = qwerasdf > 500.0 ? qwerasdf-500.0 : qwerasdf;

    //std::cout << "mouse position : (" << xpos-200 << ", " << ypos-200 << ")" << std::endl;
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
    glm::vec3 right = glm::normalize(glm::cross(cameras[0]->direction, glm::vec3(0.0, 1.0, 0.0)));
    glm::vec3 forward = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), right));
    if(keyState['i'] || keyState['I']) cameras[0]->position += 2.0f * forward;
    if(keyState['k'] || keyState['K']) cameras[0]->position -= 2.0f * forward;
    if(keyState['l'] || keyState['L']) cameras[0]->position += 2.0f * right;
    if(keyState['j'] || keyState['J']) cameras[0]->position -= 2.0f * right;
    if(main_camera == cameras[0]) {
        static double lastX = 400.0;
        static double lastY = 400.0;
        static bool firstMouse = true;
        
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        
        float delta_x = xpos - lastX;
        float delta_y = lastY - ypos; // Reversed since y-coordinates go from bottom to top
        
        lastX = xpos;
        lastY = ypos;
        
        float sensitivity = 0.002f;
        glm::vec3 right = glm::normalize(glm::cross(main_camera->direction, main_camera->up));
        glm::quat qYaw = glm::angleAxis(-delta_x * sensitivity, main_camera->up);
        glm::quat qPitch = glm::angleAxis(-delta_y * sensitivity, right);
        glm::quat q = qYaw * qPitch;

        main_camera->direction = glm::normalize(q * main_camera->direction);
    }
    
    // Check for portal teleportation after camera movement
    if (portalManager.checkAndTeleport(cameras[0]->position, cameras[0]->direction, cameras[0]->up, prevCameraPosition)) {
        // Teleportation occurred - camera position, direction, and up have been updated
    }
    
    // Store current camera position for next frame
    prevCameraPosition = cameras[0]->position;
    
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
   
    // 마우스 클릭으로 포탈 위치 설정
    static bool prevLeftMousePressed = false;
    static bool prevRightMousePressed = false;
    
    if (leftMousePressed && !prevLeftMousePressed) {
        // 왼쪽 클릭 - 포탈1 배치
        glm::vec3 hitPos, hitNormal;
        if (raycastToWall(cameras[0]->position, cameras[0]->direction, hitPos, hitNormal)) {
            // 포탈을 벽에서 약간 떨어뜨려 배치 (Z-fighting 방지)
            glm::vec3 offset = hitNormal * 10.1f;
            portalManager.setPortal1(hitPos + offset, hitNormal, glm::vec3(0.0, 1.0, 0.0));
            std::cout << "Portal 1 placed at (" << (hitPos + offset).x << ", " << (hitPos + offset).y << ", " << (hitPos + offset).z << ")" << std::endl;
        }
    }
    
    if (rightMousePressed && !prevRightMousePressed) {
        // 오른쪽 클릭 - 포탈2 배치
        glm::vec3 hitPos, hitNormal;
        if (raycastToWall(cameras[0]->position, cameras[0]->direction, hitPos, hitNormal)) {
            // 포탈을 벽에서 약간 떨어뜨려 배치 (Z-fighting 방지)
            glm::vec3 offset = hitNormal * 10.1f;
            portalManager.setPortal2(hitPos + offset, hitNormal, glm::vec3(0.0, 1.0, 0.0));
            std::cout << "Portal 2 placed at (" << (hitPos + offset).x << ", " << (hitPos + offset).y << ", " << (hitPos + offset).z << ")" << std::endl;
        }
    }
    
    prevLeftMousePressed = leftMousePressed;
    prevRightMousePressed = rightMousePressed;
    
    if (!portalManager.portal1_activated) {
        portalManager.setPortal1(glm::vec3(500.0-10.1, 200.0+10.0, 200.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    }
    if (!portalManager.portal2_activated) {
        portalManager.setPortal2(glm::vec3(200.0, 200.0, 500.0-10.1), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
    }
    portalManager.update_camera(cameras[0]->position, cameras[0]->direction, cameras[0]->up);

    Player * pl = player.get();
    pl->position = cameras[0]->position - glm::vec3(0.0, 200.0f, 0.0);
    glm::vec3 horizontalDir = glm::normalize(glm::vec3(cameras[0]->direction.x, 0.0f, cameras[0]->direction.z));
    float yaw = atan2(horizontalDir.x, horizontalDir.z) + glm::radians(90.0f);
    pl->rotation = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    SceneNode::update();
}


Test::Test() : ObjectNode("Test", glm::vec3(100.0, 100.0, 100.0), glm::vec3(0.0, 0.0, 1.0)) {
    addChild(std::make_shared<ModelNode>("portal_cube/scene", glm::vec3(0.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(0.0, 1.0, 0.0)), 10.0, glm::vec3(1.0, 1.0, 0.0)));
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

bool MainScene::raycastToWall(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& hitPos, glm::vec3& hitNormal) {
    // 방의 6개 벽 정의 (위치, 법선)
    struct Wall {
        glm::vec3 position;
        glm::vec3 normal;
        std::string name;
    };
    
    std::vector<Wall> walls = {
        {glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), "floor"},           // 바닥
        {glm::vec3(500.0, 500.0, 0.0), glm::vec3(-1.0, 0.0, 0.0), "right"},     // 오른쪽 벽
        {glm::vec3(-500.0, 500.0, 0.0), glm::vec3(1.0, 0.0, 0.0), "left"},      // 왼쪽 벽
        {glm::vec3(0.0, 500.0, -500.0), glm::vec3(0.0, 0.0, 1.0), "back"},      // 뒤쪽 벽
        {glm::vec3(0.0, 500.0, 500.0), glm::vec3(0.0, 0.0, -1.0), "front"}      // 앞쪽 벽
    };
    
    float minDistance = std::numeric_limits<float>::max();
    bool found = false;
    glm::vec3 closestHit;
    glm::vec3 closestNormal;
    
    for (const auto& wall : walls) {
        float distance;
        if (glm::intersectRayPlane(origin, direction, wall.position, wall.normal, distance)) {
            if (distance > 0.0f && distance < minDistance) {
                glm::vec3 intersection = origin + direction * distance;
                
                // 벽의 범위 내에 있는지 확인
                bool inBounds = true;
                if (wall.name == "floor") {
                    inBounds = (intersection.x >= -500.0f && intersection.x <= 500.0f &&
                               intersection.z >= -500.0f && intersection.z <= 500.0f);
                } else if (wall.name == "right" || wall.name == "left") {
                    inBounds = (intersection.y >= 0.0f && intersection.y <= 1000.0f &&
                               intersection.z >= -500.0f && intersection.z <= 500.0f);
                } else if (wall.name == "back" || wall.name == "front") {
                    inBounds = (intersection.x >= -500.0f && intersection.x <= 500.0f &&
                               intersection.y >= 0.0f && intersection.y <= 1000.0f);
                }
                
                if (inBounds) {
                    minDistance = distance;
                    closestHit = intersection;
                    closestNormal = wall.normal;
                    found = true;
                }
            }
        }
    }
    
    if (found) {
        hitPos = closestHit;
        hitNormal = closestNormal;
        return true;
    }
    
    return false;
}