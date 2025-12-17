#include "portal.h"

#include <map>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <stack>

extern glm::mat4 view_matrix;
extern glm::mat4 projection_matrix;
extern glm::mat4 model_matrix;
extern std::stack<glm::mat4> stack_model_matrix;

extern GLint clipPlaneLoc;

Portal::Portal() : position(glm::vec3(1.0)), direction(glm::vec3(1.0)), up(glm::vec3(1.0)) {
    stencil_model = std::make_shared<ModelNode>("portal", glm::vec3(0.0, 0.0, 0.0), glm::angleAxis(float(glm::radians(90.0)), glm::vec3(1.0, 0.0, 0.0)), 100.0f, glm::vec3(0.0, 1.0, 0.0));
}

void Portal::redner_stencil_model() {
    stack_model_matrix.push(model_matrix);

    model_matrix = local_to_wolrd();
    stencil_model->render();

    model_matrix = stack_model_matrix.top();
    stack_model_matrix.pop();
}

void Portal::redner_stencil_model(const glm::mat4& m) {
    stack_model_matrix.push(model_matrix);

    model_matrix = local_to_wolrd();
    stencil_model->render();

    model_matrix = stack_model_matrix.top();
    stack_model_matrix.pop();
}

bool Portal::isPositionBehind(const glm::vec3& pos) const {
    // Vector from portal position to test position
    glm::vec3 toPos = pos - position;
    // If dot product with portal direction is negative, position is behind portal
    return glm::dot(toPos, direction) < 0.0f;
}

bool Portal::didCrossPortal(const glm::vec3& prevPos, const glm::vec3& currPos) const {
    // Check if the two positions are on opposite sides of the portal plane
    bool prevBehind = isPositionBehind(prevPos);
    bool currBehind = isPositionBehind(currPos);
    // Crossing occurred if we moved from front (not behind) to back (behind)
    // This means we went THROUGH the portal
    return !prevBehind && currBehind;
}

void PortalManager::setPortal1(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u) {
    portal1.position = pos;
    portal1.direction = dir;
    portal1.up = u;

    portal1_activated = true;
}

void PortalManager::setPortal2(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u) {
    portal2.position = pos;
    portal2.direction = dir;
    portal2.up = u;

    portal2_activated = true;
}

void PortalManager::update_camera(const glm::vec3& viewer_position, const glm::vec3& viewer_direction, const glm::vec3& viewer_up) {
    if(!portal1_activated || !portal2_activated) return;
    /*
    glm::mat4 portal1World = portal1.local_to_wolrd();
    glm::mat4 portal1World_inverse = glm::inverse(portal1World);
    glm::mat4 portal2World = portal2.local_to_wolrd();
    glm::mat4 portal2World_inverse = glm::inverse(portal2World);
    glm::mat4 flip180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0,1,0));

    portal1_camera->position = glm::vec3(portal2World * flip180 * portal1World_inverse * glm::vec4(viewer_position, 1.0));
    portal2_camera->position = glm::vec3(portal1World * flip180 * portal2World_inverse * glm::vec4(viewer_position, 1.0));

    portal1_camera->direction = glm::mat3(portal2World * flip180 * portal1World_inverse) * viewer_direction;
    portal2_camera->direction = glm::mat3(portal1World * flip180 * portal2World_inverse) * viewer_direction;

    portal1_camera->up = glm::mat3(portal2World * flip180 * portal1World_inverse) * viewer_up;
    portal2_camera->up = glm::mat3(portal1World * flip180 * portal2World_inverse) * viewer_up;
*/
    portal1_camera->position = viewer_position;
    portal2_camera->position = viewer_position;
    portal1_camera->direction = viewer_direction;
    portal2_camera->direction = viewer_direction;
    portal1_camera->up = viewer_up;
    portal2_camera->up = viewer_up;
}

PortalManager::PortalManager() : portal1(), portal2() {
    //일단 portal의 카메라 설정(fov, 비율 등) 하드 코딩함
    portal1_camera = std::make_shared<PersCameraNode>("portal1_camera", glm::vec3(-300.0, 200.0f, -300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f, 1.0f);
    portal2_camera = std::make_shared<PersCameraNode>("portal2_camera", glm::vec3(-300.0, 200.0f, -300.0), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 00), 120.0f, 1.0f, 3000.0f, 1.0f);
}

void PortalManager::draw_stencil() {
    //stencil buffer를 만들고 해당 영역의 색상과 깊이를 초기화
    
    //행렬은 world coorindate로 맞춰져 있다고 생각함


    // --- 스텐실 영역 그리기 portal1 => 1로, portal2 => 2로
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 화면엔 안 보이게 색상 끔
    glDepthMask(GL_FALSE);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 통과 시 기준값으로 설정

    //portal1
    glStencilFunc(GL_ALWAYS, 1, 0xFF);         // 무조건 통과, 값은 1
    portal1.redner_stencil_model();

    //portal2
    glStencilFunc(GL_ALWAYS, 2, 0xFF);         // 무조건 통과, 값은 2
    portal2.redner_stencil_model();

    // --- stencil 영역의 생상, 깊이 초기화
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glDepthRange(1.0, 1.0);
    glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);    // 이제 버퍼 값은 바꾸지 말고 유지

    //portal1
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    portal1.redner_stencil_model();

    //portal2
    glStencilFunc(GL_EQUAL, 2, 0xFF);
    portal2.redner_stencil_model();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthRange(0.0, 1.0);
    glDepthFunc(GL_LESS);
}


void PortalManager::setPortal1Clipping(float offset) {
    glm::vec3 planeNormal;
    glm::vec3 planePoint;
    glm::vec4 clipPlane;

    planeNormal = portal2.direction;
    planePoint  = portal2.position + offset*portal2.direction;
    clipPlane = glm::vec4(planeNormal.x, planeNormal.y, planeNormal.z,
                        -glm::dot(planeNormal, planePoint)  // D = -n·p
    );
    glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
}
void PortalManager::setPortal2Clipping(float offset) {
    glm::vec3 planeNormal;
    glm::vec3 planePoint;
    glm::vec4 clipPlane;

    planeNormal = portal1.direction;
    planePoint  = portal1.position + offset*portal1.direction;
    clipPlane = glm::vec4(planeNormal.x, planeNormal.y, planeNormal.z,
                        -glm::dot(planeNormal, planePoint)  // D = -n·p
    );
    glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
}

void PortalManager::setPortal1Clipping(const glm::mat4& portal1_matrix, float offset) {
    glm::vec3 planeNormal;
    glm::vec3 planePoint;
    glm::vec4 clipPlane;

    planeNormal = glm::mat3(portal1_matrix)*portal2.direction;
    planePoint  = glm::vec3(portal1_matrix*glm::vec4(portal2.position, 1.0)) + offset*planeNormal;
    clipPlane = glm::vec4(planeNormal.x, planeNormal.y, planeNormal.z,
                        -glm::dot(planeNormal, planePoint)  // D = -n·p
    );
    glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));

}
void PortalManager::setPortal2Clipping(const glm::mat4& portal2_matrix, float offset) {
    glm::vec3 planeNormal;
    glm::vec3 planePoint;
    glm::vec4 clipPlane;

    planeNormal = glm::mat3(portal2_matrix)*portal1.direction;
    planePoint  = glm::vec3(portal2_matrix*glm::vec4(portal1.position, 1.0)) + offset*planeNormal;
    clipPlane = glm::vec4(planeNormal.x, planeNormal.y, planeNormal.z,
                        -glm::dot(planeNormal, planePoint)  // D = -n·p
    );
    glUniform4fv(clipPlaneLoc, 1, glm::value_ptr(clipPlane));
}

void PortalManager::draw_stencil(int depth, const glm::mat4& portal1_matrix, const glm::mat4& portal2_matrix) {
    //stencil buffer를 만들고 해당 영역의 색상과 깊이를 초기화
    
    //행렬은 world coorindate로 맞춰져 있다고 생각함
    //depth 0 -> 1,  128+1
    //depth i -> i+1, 128+i+1

    // --- 스텐실 영역 그리기 portal1 => 1로, portal2 => 2로
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 화면엔 안 보이게 색상 끔
    glDepthMask(GL_FALSE);
    glStencilMask(0xFF);

    //portal1
    if(depth == 0) {
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
    }
    else {
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glStencilFunc(GL_EQUAL, depth, 0xFF);
    }
    portal1_camera->setup();
    portal1.redner_stencil_model(portal1_matrix);

    //portal2
    if(depth == 0) {
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 128+1, 0xFF);
    }
    else {
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glStencilFunc(GL_EQUAL, 128+depth, 0xFF);
    }
    portal2_camera->setup();
    portal2.redner_stencil_model(portal2_matrix);

    // --- stencil 영역의 생상, 깊이 초기화
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glDepthRange(1.0, 1.0);
    glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);    // 이제 버퍼 값은 바꾸지 말고 유지

    //portal1
    glStencilFunc(GL_EQUAL, depth+1, 0xFF);
    portal1_camera->setup();
    portal1.redner_stencil_model(portal1_matrix);

    //portal2
    glStencilFunc(GL_EQUAL, 128+depth+1, 0xFF);
    portal2_camera->setup();
    portal2.redner_stencil_model(portal2_matrix);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthRange(0.0, 1.0);
    glDepthFunc(GL_LESS);
}

bool PortalManager::checkAndTeleport(glm::vec3& position, glm::vec3& direction, glm::vec3& up, const glm::vec3& prevPosition) {
    // Both portals must be activated for teleportation
    if (!portal1_activated || !portal2_activated) {
        return false;
    }
    
    // Check if crossed portal 1 -> teleport to portal 2
    if (portal1.didCrossPortal(prevPosition, position)) {
        std::cout << "Portal 1 crossed! Teleporting to Portal 2..." << std::endl;
        // Transform matrices
        glm::mat4 flip180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 m12 = portal2.local_to_wolrd() * flip180 * portal1.local_to_wolrd_inverse();
        
        // Transform position
        glm::vec4 newPos = m12 * glm::vec4(position, 1.0f);
        position = glm::vec3(newPos) / newPos.w;
        
        // Transform direction and up vectors (rotation only, no translation)
        glm::mat3 rotationMatrix = glm::mat3(m12);
        direction = glm::normalize(rotationMatrix * direction);
        up = glm::normalize(rotationMatrix * up);
        
        return true;
    }
    
    // Check if crossed portal 2 -> teleport to portal 1
    if (portal2.didCrossPortal(prevPosition, position)) {
        std::cout << "Portal 2 crossed! Teleporting to Portal 1..." << std::endl;
        // Transform matrices
        glm::mat4 flip180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 m21 = portal1.local_to_wolrd() * flip180 * portal2.local_to_wolrd_inverse();
        
        // Transform position
        glm::vec4 newPos = m21 * glm::vec4(position, 1.0f);
        position = glm::vec3(newPos) / newPos.w;
        
        // Transform direction and up vectors (rotation only, no translation)
        glm::mat3 rotationMatrix = glm::mat3(m21);
        direction = glm::normalize(rotationMatrix * direction);
        up = glm::normalize(rotationMatrix * up);
        
        return true;
    }
    
    return false;
}