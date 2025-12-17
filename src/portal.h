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
    void redner_stencil_model(const glm::mat4& m);
    
    // Check if a position is behind the portal plane (in the direction opposite to portal's facing)
    bool isPositionBehind(const glm::vec3& pos) const;
    // Check if movement from prevPos to currPos crossed the portal plane
    bool didCrossPortal(const glm::vec3& prevPos, const glm::vec3& currPos) const;
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
    void setPortal1Clipping(const glm::mat4& portal1_matrix, float offset=0.0f);
    void setPortal2Clipping(float offset=0.0f);
    void setPortal2Clipping(const glm::mat4& portal2_matrix, float offset=0.0f);

    void update_camera(const glm::vec3& viewer_position, const glm::vec3& viewer_direction, const glm::vec3& viewer_up);

    void draw_stencil();
    void draw_stencil(int depth, const glm::mat4& portal1_matrix, const glm::mat4& portal2_matrix);

    void render();
    
    // Check if position crossed a portal and teleport if needed
    // Returns true if teleportation occurred
    bool checkAndTeleport(glm::vec3& position, glm::vec3& direction, glm::vec3& up, const glm::vec3& prevPosition);
};