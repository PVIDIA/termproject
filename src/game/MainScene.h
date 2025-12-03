#include "../node.h"

#include <vector>

class PortalTest : public ObjectNode {
public:
    glm::vec3 portal1_position;
    glm::vec3 portal1_direction;
    glm::vec3 portal1_up;
    
    glm::vec3 portal2_position;
    glm::vec3 portal2_direction;
    glm::vec3 portal2_up;
    
    std::shared_ptr<CameraNode> p1_camera;
    std::shared_ptr<CameraNode> p2_camera;

    GLuint p1_texture;
    GLuint p2_texture;
    
    GLuint p1_FBO;
    GLuint p2_FBO;

    PortalTest();
    void update(const glm::vec3& p);
    void update() {}
    void render() {}
}

class MainScene : public SceneNode {
public:
    std::vector<std::shared_ptr<CameraNode>> cameras;
    int current_camera_num;

    std::shared_ptr<ObjectNode> player;

    PortalTest portalTest;
    glm::vec3 portal1_position;
    glm::vec3 portal1_direction;
    glm::vec3 portal1_up;
    
    glm::vec3 portal2_position;
    glm::vec3 portal2_direction;
    glm::vec3 portal2_up;

    std::shared_ptr<CameraNode> p1_camera;
    std::shared_ptr<CameraNode> p2_camera;

    glm::vec3 player_position;

    int time = 0;

    MainScene();
    void update();
    void render();
};

class Test : public ObjectNode {
public:
    Test();
    void update();
    void render();
};
