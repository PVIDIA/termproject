#include "node.h"

#include <cmath>
#include <algorithm>
#include <map>
#include <utility>
#include <stack>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

glm::mat4 model_matrix(1.0f);
glm::mat4 view_matrix(1.0f);
glm::mat4 projection_matrix(1.0f);
std::stack<glm::mat4> stack_model_matrix;

extern GLint projLoc;
extern GLint viewLoc;
extern GLint modelLoc;
extern GLint colorLoc;
extern GLint cameraLoc;
extern GLint pointLightLoc;

extern ModelManager modelManager;

void glTranformate(glm::vec3 position, glm::vec3 direction, float scale, float angle_) {
    glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 dir = glm::normalize(direction);

    float cosTheta = glm::dot(z, dir);
    cosTheta = std::clamp(cosTheta, -1.0f, 1.0f);
    float angle = std::acos(cosTheta);

    glm::vec3 rotate_axis = glm::cross(z, dir);

    glTranslatef(position.x, position.y, position.z);
    if(angle > 0.000000001 || angle < -0.000000001) glRotatef(angle*180.0f/glm::pi<float>(), rotate_axis.x, rotate_axis.y, rotate_axis.z);
    glRotatef(angle_, 0.0, 0.0, 1.0);
    glScalef(scale, scale, scale);
}

glm::quat dir_to_quat(const glm::vec3& direction, float angle_) {
    if(direction == glm::vec3(0.0f, 0.0f, 1.0f)) {
        return glm::quat(1.0, 0.0, 0.0, 0.0);
    }
    else if(direction == glm::vec3(0.0f, 0.0f, -1.0f)) {
        return glm::quat(0.0, 1.0, 0.0, 0.0);
    }
    else {
        return glm::normalize(glm::quat(1+direction.x, -direction.y, direction.x, 0.0));
    }

    glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 dir = glm::normalize(direction);

    float cosTheta = glm::dot(z, dir);
    cosTheta = std::clamp(cosTheta, -1.0f, 1.0f);
    float angle = std::acos(cosTheta);

    glm::vec3 rotate_axis = glm::normalize(glm::cross(z, dir));

    glm::quat q(1.0, 0.0, 0.0, 0.0);
    if(angle > 0.000000001 || angle < -0.000000001) q = q*glm::angleAxis(angle, rotate_axis);
    q = q*glm::angleAxis(angle_, glm::vec3(0.0, 0.0, 1.0));

    return q;
}

glm::vec3 quat_to_dir(const glm::quat& q) {
    glm::vec4 dir4 = glm::toMat4(q) * glm::vec4(0.0f, 0.0f, 1.0f, 1.0);
    return glm::vec3(dir4);
}

//__________  Scene __________
SceneNode::SceneNode(const std::string& n) : Node(n) {
    axes = std::make_shared<ObjectNode>("axes", glm::vec3(0.0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    axes->addChild(std::make_shared<ModelNode>("Axis", glm::vec3(0.0), glm::angleAxis(glm::radians(0.0f), glm::vec3(0,0,1)), 1.0f, glm::vec4(0.2, 0.2, 1.0, 0.7)));
    axes->addChild(std::make_shared<ModelNode>("Axis", glm::vec3(0.0), glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0)), 1.0f, glm::vec4(1.0, 0.2, 0.2, 0.7)));
    axes->addChild(std::make_shared<ModelNode>("Axis", glm::vec3(0.0), glm::angleAxis(glm::radians(-90.0f), glm::vec3(1,0,0)), 1.0f, glm::vec4(0.2, 1.0, 0.2, 0.7)));
}


void SceneNode::update(){ {}
    kill();
    detach();

    model_matrix = glm::mat4(1.0f);
    stack_model_matrix = std::stack<glm::mat4>();

    for(auto& node : children) {
        //node 업데이트
        stack_model_matrix.push(model_matrix);
        node->update();
        model_matrix = stack_model_matrix.top();
        stack_model_matrix.pop();
    }
    
    //충돌 처리
    collide();
}

void SceneNode::render() {
    //background

    //camera
    //std::cout << "camera setup" << std::endl;
    main_camera->setup();
    
    model_matrix = glm::mat4(1.0f);
    stack_model_matrix = std::stack<glm::mat4>();
    
    axes->render();

    for(auto& node : children) {
        //node 업데이트
        stack_model_matrix.push(model_matrix);
        node->render();
        model_matrix = stack_model_matrix.top();
        stack_model_matrix.pop();
    }
}

bool is_collided(const CollisionNode& a, const CollisionNode& b) {
    glm::vec3 delta = a.real_position - b.real_position;
    return (a.radius+b.radius)*(a.radius+b.radius) > (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
}

void SceneNode::collide() {
    collisions.clear();
    collectCollisions();
    std::map<std::pair<ObjectNode*, ObjectNode*>, bool> boolMap;

    for (auto it1 = collisions.begin(); it1 != collisions.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != collisions.end(); ++it2) {
            std::shared_ptr<CollisionNode>& a = *it1;
            std::shared_ptr<CollisionNode>& b = *it2;
            if(a->parent == nullptr || b->parent == nullptr || boolMap[{a->parent, b->parent}] || a->inactive || b->inactive || a->parent->independent || b->parent->independent) continue;

            if(is_collided(*a, *b)) {
                a->parent->attacked(b->parent);
                b->parent->attacked(a->parent);

                boolMap[{a->parent, b->parent}] = true;
                boolMap[{b->parent, a->parent}] = true;
            }
        }
    }
}

void collectHelp(std::vector<std::shared_ptr<CollisionNode>>& c, std::shared_ptr<Node>& node) {
    if(node->getType() == "CollisionNode") {
        auto collision_ptr = std::dynamic_pointer_cast<CollisionNode>(node);
        if(collision_ptr) {
            c.push_back(collision_ptr);
        }
    }

    for(std::shared_ptr<Node>& child : node->children)
        collectHelp(c, child);
}
void SceneNode::collectCollisions() {
    for(std::shared_ptr<Node>& child : children)
        collectHelp(collisions, child);
}

int scene_node_count;

void killHelp(Node* node) {
    scene_node_count++;
    for(auto it = node->children.begin(); it != node->children.end();) {
        killHelp((*it).get());
        std::shared_ptr<Node>& child = *it;

        bool erased = false;
        if(child->getType() == "ObjectNode") {
            auto object = std::dynamic_pointer_cast<ObjectNode>(child);
            if(object->toDelete) {
                //std::cout << "child deleted : " << child->name << std::endl;
                it = node->children.erase(it);
                erased = true;
            }
        }
        if(!erased) ++it;
    }
}

void SceneNode::kill() {
    scene_node_count = 0;
    killHelp(this);
    //std::cout << "scene_node_count : " << scene_node_count << std::endl;
    //std::cout << "scnen chilred count : " << children.size() << std::endl;
}

void detachHelp(SceneNode& scene, std::shared_ptr<Node>& node, std::vector<std::shared_ptr<Node>>& erased_nodes, const glm::mat4& transformation) {
    glm::mat4 tf = transformation*glm::translate(glm::mat4(1.0f), node->position)*glm::toMat4(node->rotation);
    for(auto it = node->children.begin(); it != node->children.end();) {
        detachHelp(scene, *it, erased_nodes, tf);
        std::shared_ptr<Node>& child = *it;

        bool erased = false;
        if(child->getType() == "ObjectNode") {
            auto object = std::dynamic_pointer_cast<ObjectNode>(child);
            if(object->toDelete) {
                //std::cout << "child deleted : " << child->name << std::endl;
                it = node->children.erase(it);
                erased = true;
            }
            else if(object->independent) {
                //std::cout << "node : " << node->name << std::endl;
                //std::cout << "child : " << child->name << std::endl;
                object->independent = false;
                object->position = glm::vec3(tf*glm::vec4(object->position, 1.0f));

                glm::mat3 rotation_mat3(tf); 
                rotation_mat3[0] = glm::normalize(rotation_mat3[0]);
                rotation_mat3[1] = glm::normalize(rotation_mat3[1]);
                rotation_mat3[2] = glm::normalize(rotation_mat3[2]);
                object->rotation = glm::quat(rotation_mat3)*object->rotation;
                
                erased_nodes.push_back(object);
                it = node->children.erase(it);
                erased = true;
            }
        }

        if(!erased) ++it;
    }
}
void SceneNode::detach() {
    std::vector<std::shared_ptr<Node>> erased_nodes;
    for(auto& child : children) {
        detachHelp(*this, child, erased_nodes, glm::mat4(1.0f));
    }
    for(auto& child : erased_nodes) {
        child->invisible = false;
        child->inactive = false;
        addChild(child);
    }
}

//__________ PersCamera __________

void PersCameraNode::update() {
    transformation = model_matrix;
}

void PersCameraNode::render() {
}

void PersCameraNode::setup() {
    //Projection은 바로 적용
    //projection_matrix = glm::perspective(fovy, 500.0f/750.0f, zNear, zFar);
    projection_matrix = glm::perspective(fovy, aspect, zNear, zFar);

    glm::mat4 model_transformation = transformation;
    glm::mat3 model_rotation = glm::mat3(model_transformation);
    
    //Camera에 좌표에 model view 변환을 적용
    glm::vec4 translated_pos_4 = model_transformation * glm::vec4(position, 1.0);
    glm::vec3 translated_pos = glm::vec3(translated_pos_4) / translated_pos_4.w;
    glUniform3fv(cameraLoc, 1, glm::value_ptr(translated_pos)); //GLSL camera position setting

    glm::vec3 translated_dir = model_rotation * direction;
    glm::vec3 translated_up = model_rotation * up;

    glm::vec3 translated_at = translated_pos + translated_dir;

    //Camera 세팅
    view_matrix = glm::lookAt(translated_pos, translated_at, translated_up);
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
    /*
    if(name == "PlayerPersCamera") {
        std::cout << "=== PlayerPersCamera ===" << std::endl;
        std::cout << "- transformation -" << std::endl;
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                std::cout << transformation[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "-----" << std::endl;
        std::cout << "position : " << position.x << ", " << position.y << ", " << position.z << std::endl;
        std::cout << "direction : " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
        std::cout << "up : " << up.x << ", " << up.y << ", " << up.z << std::endl;
        std::cout << "-----" << std::endl;
        std::cout << "t_position : " << translated_pos.x << ", " << translated_pos.y << ", " << translated_pos.z << std::endl;
        std::cout << "t_direction : " << translated_dir.x << ", " << translated_dir.y << ", " << translated_dir.z << std::endl;
        std::cout << "t_up : " << translated_up.x << ", " << translated_up.y << ", " << translated_up.z << std::endl;
        std::cout << "-----" << std::endl;
        std::cout << "=====" << std::endl;
    }*/
}


void OrthoCameraNode::update() {
    transformation = model_matrix;
}
void OrthoCameraNode::render() {

}
void OrthoCameraNode::setup() {
    //Projection은 바로 적용
    projection_matrix = glm::ortho(-half_width, half_width, -half_height, half_height, 0.001f, zFar);
    
    glm::mat4 model_transformation = transformation;
    glm::mat3 model_rotation = glm::mat3(model_transformation);
    
    //Camera에 좌표에 model view 변환을 적용
    glm::vec4 translated_pos_4 = model_transformation * glm::vec4(position, 1.0);
    glm::vec3 translated_pos = glm::vec3(translated_pos_4) / translated_pos_4.w;
    glm::vec3 translated_dir = model_rotation * direction;
    glm::vec3 translated_up = model_rotation * up;

    glm::vec3 translated_at = translated_pos + translated_dir;

    //Camera 세팅
    view_matrix = glm::lookAt(translated_pos, translated_at, translated_up);
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
}

// __________ Moodel __________

ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec4& c, const std::string& t, const std::string& nt)
            : Node(n, pos, q), scale(s), color(c) {
    modelManager.loadModel(n, t, nt);
}

ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec3& c, const std::string& t, const std::string& nt)
            : Node(n, pos, q), scale(s), color(glm::vec4(c, 1.0)) {
    modelManager.loadModel(n, t, nt);
}

ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, float s, const glm::vec4& c, const std::string& t, const std::string& nt)
            : Node(n, pos, dir_to_quat(dir)), scale(s), color(c) {
    modelManager.loadModel(n, t, nt);
}
ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, float s, const glm::vec3& c, const std::string& t, const std::string& nt)
            : Node(n, pos, dir_to_quat(dir)), scale(s), color(glm::vec4(c, 1.0)) {
    modelManager.loadModel(n, t, nt);
}

ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec3& c, const std::string& t, GLuint tt)
            : Node(n, pos, q), scale(s), color(glm::vec4(c, 1.0)) {
    modelManager.loadModel(n, "", "");
    modelManager.at(n).texture = t;
    modelManager.textures[t] = tt;
}

void ModelNode::update() {
}

void ModelNode::render() {
    if(invisible) return;

    model_matrix = model_matrix * glm::translate(glm::mat4(1.0), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0), glm::vec3(scale));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniform4fv(colorLoc, 1, glm::value_ptr(color));
    modelManager.draw_model(name);
}

// __________ Object __________
ObjectNode::ObjectNode(const std::string& n, const glm::vec3& pos, const glm::quat& rot) : Node(n, pos, rot) {}

ObjectNode::ObjectNode(const std::string& n, const glm::vec3& pos, const glm::vec3 dir) : Node(n, pos, dir_to_quat(dir)) {}

void ObjectNode::update() {
    //rotation = rotation * glm::angleAxis(0.01f, glm::vec3(0.0, 1.0, 0.0));

    if(inactive) return;

    model_matrix = model_matrix * glm::translate(glm::mat4(1.0), position) * glm::toMat4(rotation);
    for(auto& node : children) {
        stack_model_matrix.push(model_matrix);
        node->update();
        model_matrix = stack_model_matrix.top();
        stack_model_matrix.pop();
    }
}

void ObjectNode::render() {
    if(invisible) return;

    model_matrix = model_matrix * glm::translate(glm::mat4(1.0), position) * glm::toMat4(rotation);
    for(auto& node : children) {
        stack_model_matrix.push(model_matrix);
        node->render();
        model_matrix = stack_model_matrix.top();
        stack_model_matrix.pop();
    }
}

// __________ Collision __________
CollisionNode::CollisionNode(const std::string& n, const glm::vec3& pos, float r, ObjectNode* p)
                : Node(n, pos), radius(r), parent(p) {
}

void CollisionNode::update() {
    auto pos = model_matrix * glm::vec4(position, 1.0f);
    real_position = glm::vec3(pos.x/pos.w, pos.y/pos.w, pos.z/pos.w);
}

bool render_collision;
void CollisionNode::render() {
    if(!render_collision) return;
    
    model_matrix = model_matrix * glm::translate(glm::mat4(1.0), position) * glm::scale(glm::mat4(1.0), glm::vec3(radius));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 0.5);
    modelManager.loadModel("Sphere");
    modelManager.draw_model("Sphere");
}