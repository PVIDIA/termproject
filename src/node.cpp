#include "node.h"

#include <cmath>
#include <algorithm>
#include <map>
#include <utility>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>



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

glm::vec3 transformate_to_world(const glm::vec3& v) {
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    auto transformation = glm::make_mat4(mat);
    glm::vec4 v_ = transformation * glm::vec4(v, 1.0);
    return glm::vec3(v_.x/v_.w, v_.y/v_.w, v_.z/v_.w);
}

//__________  Scene __________

void SceneNode::update(){ {}
    kill();
    detach();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for(auto& node : children) {
        //node 업데이트
        glPushMatrix();
        node->update();
        glPopMatrix();
    }
    
    //충돌 처리
    collide();
}

void SceneNode::render() {
    //background

    //camera
    //std::cout << "camera setup" << std::endl;
    main_camera->setup();
    
    //x-aixs
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(10000000000.0f, 0.0f, 0.0f);
    glEnd();

    //y-aixs
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 10000000000.0f, 0.0f);
    glEnd();

    //z-aixs
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 10000000000.0f);
    glEnd();

    glMatrixMode(GL_MODELVIEW);
    for(auto& node : children) {
        //node 업데이트
        glPushMatrix();
        //std::cout << "render : " << node->type << " : " << node->name << std::endl;
        node->render();
        glPopMatrix();
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
    std::cout << "scene_node_count : " << scene_node_count << std::endl;
    //std::cout << "scnen chilred count : " << children.size() << std::endl;
}

void detachHelp(SceneNode& scene, std::shared_ptr<Node>& node, std::vector<std::shared_ptr<Node>>& erased_nodes) {
    for(auto it = node->children.begin(); it != node->children.end();) {
        detachHelp(scene, *it, erased_nodes);
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
        detachHelp(*this, child, erased_nodes);
    }
    for(auto& child : erased_nodes) {
        child->invisible = false;
        child->inactive = false;
        addChild(child);
    }
}

//__________ PersCamera __________

void PersCameraNode::update() {
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    transformation = glm::make_mat4(mat);
}

void PersCameraNode::render() {
}

void PersCameraNode::setup() {
    //Projection은 바로 적용
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Replace gluPerspective with glm::perspective
    float aspect = 500.0f / 750.0f;
    glm::mat4 projection = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
    glLoadMatrixf(glm::value_ptr(projection));
    
    glm::mat4 model_transformation = transformation;
    glm::mat3 model_rotation = glm::mat3(model_transformation);
    
    //Camera에 좌표에 model view 변환을 적용
    glm::vec4 translated_pos_4 = model_transformation * glm::vec4(position, 1.0);
    glm::vec3 translated_pos = glm::vec3(translated_pos_4) / translated_pos_4.w;
    glm::vec3 translated_dir = model_rotation * direction;
    glm::vec3 translated_up = model_rotation * up;

    glm::vec3 translated_at = translated_pos + translated_dir;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Replace gluLookAt with glm::lookAt
    glm::mat4 view = glm::lookAt(translated_pos, translated_at, translated_up);
    glLoadMatrixf(glm::value_ptr(view));
}


void OrthoCameraNode::update() {
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    transformation = glm::make_mat4(mat);
}
void OrthoCameraNode::render() {

}
void OrthoCameraNode::setup() {
    //Projection은 바로 적용
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-half_width, half_width, -half_height, half_height, 0.001, zFar);
    
    glm::mat4 model_transformation = transformation;
    glm::mat3 model_rotation = glm::mat3(model_transformation);
    
    //Camera에 좌표에 model view 변환을 적용
    glm::vec4 translated_pos_4 = model_transformation * glm::vec4(position, 1.0);
    glm::vec3 translated_pos = glm::vec3(translated_pos_4) / translated_pos_4.w;
    glm::vec3 translated_dir = model_rotation * direction;
    glm::vec3 translated_up = model_rotation * up;

    glm::vec3 translated_at = translated_pos + translated_dir;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Replace gluLookAt with glm::lookAt
    glm::mat4 view = glm::lookAt(translated_pos, translated_at, translated_up);
    glLoadMatrixf(glm::value_ptr(view));
}

// __________ Moodel __________

ModelNode::ModelNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, float s, const glm::vec3& c, float ang)
            : Node(n), position(pos), direction(dir), scale(s), color(c), angle(ang) {
    modelManager.loadModel(n);
}

void ModelNode::update() {
}

void ModelNode::render() {
    if(invisible) return;

    glTranformate(position, direction, scale, angle);
    glColor3f(color.x, color.y, color.z);
    modelManager.draw_model(name);
}

// __________ Object __________


ObjectNode::ObjectNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir) : Node(n), position(pos), direction(dir) {

}

void ObjectNode::update() {
    if(inactive) return;

    glTranformate(position, direction);
    for(auto& node : children) {
        //node 업데이트
        glPushMatrix();
        node->update();
        glPopMatrix();
    }
}

void ObjectNode::render() {
    if(invisible) return;

    glTranformate(position, direction);
    for(auto& node : children) {
        //node 업데이트
        glPushMatrix();
        node->render();
        glPopMatrix();
    }
}

// __________ Collision __________

CollisionNode::CollisionNode(const std::string& n, const glm::vec3& pos, float r, ObjectNode* p)
                : Node(n), position(pos), radius(r), parent(p) {
}

void CollisionNode::update() {
    glTranslatef(position.x, position.y, position.z);
    
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    auto transformation = glm::make_mat4(mat);
    glm::vec4 origin(0.0f, 0.0f, 0.0f, 1.0f);

    real_position = glm::vec3(transformation * origin);
}

bool render_collision;
void CollisionNode::render() {
    if(!render_collision) return;
    /*
    glTranslatef(position.x, position.y, position.z);
    glColor4f(1.0f, 0.0f, 0.0f, 0.5);
    glutWireSphere(radius, 10, 10);
    */
}