#include "Player.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

extern GLFWwindow* window;
extern bool isRenderingPortal;

Player::Player() : ObjectNode("Player", glm::vec3(100.0, 0.0, 100.0), glm::vec3(0.0, 0.0, 1.0)) {
    addChild(std::make_shared<ModelNode>("chell/scene", glm::vec3(0.0), glm::angleAxis(float(glm::radians(-90.0)), glm::vec3(0.0, 1.0, 0.0)), 3.0, glm::vec3(1.0, 1.0, 0.0)));
    previous_position = position;
}

void Player::update() {
    // Store previous position before updating
    previous_position = position;
    
    ObjectNode::update();
}
void Player::render() {
    if (isRenderingPortal) {
        ObjectNode::render();
    }
}
