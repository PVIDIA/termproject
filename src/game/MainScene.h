#include "../node.h"
#include "../portal.h"

#include <vector>

class MainScene : public SceneNode {
public:
    std::vector<std::shared_ptr<CameraNode>> cameras;
    int current_camera_num;

    PortalManager portalManager;

    int time = 0;

    MainScene();
    void update();
    void render();
    void render_recursive(int depth, int max_depth, const glm::mat4& portal1_matrix, const glm::mat4& portal2_matrix, const glm::mat4& m12, const glm::mat4& m21);
};

class Test : public ObjectNode {
public:
    Test();
    void update();
    void render();
};
