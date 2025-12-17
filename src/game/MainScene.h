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
};

class Test : public ObjectNode {
public:
    Test();
    void update();
    void render();
};
