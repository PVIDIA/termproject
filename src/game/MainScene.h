#include "../node.h"
#include "../portal.h"
#include "Player.h"

#include <vector>

class MainScene : public SceneNode {
public:
    std::vector<std::shared_ptr<CameraNode>> cameras;
    int current_camera_num;

    PortalManager portalManager;
    std::shared_ptr<Player> player;

    int time = 0;

    MainScene();
    void update();
    void render();
    void render_recursive(int depth, int max_depth, const glm::mat4& portal1_matrix, const glm::mat4& portal2_matrix, const glm::mat4& m12, const glm::mat4& m21);
    bool raycastToWall(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& hitPos, glm::vec3& hitNormal);
};

class Test : public ObjectNode {
public:
    Test();
    void update();
    void render();
};
