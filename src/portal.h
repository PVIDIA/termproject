#pragma once
#include "node.h"

#include <vector>

class Portal {
public:
    glm::vec3 position, direction, up;

    Portal();

    glm::mat4 local_to_wolrd() {
        return glm::mat4(
                            glm::vec4(glm::cross(up, direction), 0.0f),
                            glm::vec4(up, 0.0f),
                            glm::vec4(direction, 0.0f),
                            glm::vec4(position, 1.0f)
                        );
    }
    
    glm::mat4 local_to_wolrd_inverse() {
        return glm::inverse(local_to_wolrd());
    }

    std::shared_ptr<ModelNode> stencil_model;
    std::shared_ptr<ModelNode> texture_model;

    void redner_stencil_model();
};

class PortalManager {
public:
    Portal portal1; bool portal1_activated;
    Portal portal2; bool portal2_activated;
    
    std::shared_ptr<CameraNode> portal1_camera;
    std::shared_ptr<CameraNode> portal2_camera;

    PortalManager();
    PortalManager(const CameraNode& cameraNode);

    void setPortal1(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u);
    void setPortal2(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u);
    
    void setPortal1Clipping(float offset=0.0f);
    void setPortal2Clipping(float offset=0.0f);

    void update_camera(const glm::vec3& viewer_position, const glm::vec3& viewer_direction, const glm::vec3& viewer_up);

    void draw_stencil();

    void render();
};