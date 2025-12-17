#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "model.h"

glm::quat dir_to_quat(const glm::vec3& direction, float angle_=0.0f);
glm::vec3 quat_to_dir(const glm::quat& q);

class CollisionNode;
class ObjectNode;

class Node {
public:
    bool invisible = false;
    bool inactive = false;

    std::string name;
    std::string type = "Node";
    std::vector<std::shared_ptr<Node>> children;
    
    glm::vec3 position;
    glm::quat rotation;


    Node(const std::string& n) : name(n), position(glm::vec3(0.0)), rotation(glm::quat(1.0, 0.0, 0.0, 0.0)) {}
    Node(const std::string& n, const glm::vec3& pos, const glm::quat& q=glm::quat(1.0, 0.0, 0.0, 0.0)) : name(n), position(pos), rotation(q) {}

    void addChild(std::shared_ptr<Node> n) { children.push_back(n); }
    
    virtual std::string getType() const { return type; } 
    virtual void update() { if(inactive) return; }
    virtual void render() { if(invisible) return; }
};

//__________ Camera __________

class CameraNode : public Node {
public:
    std::string type = "CameraNode";

    glm::vec3 direction;
    glm::vec3 up;

    glm::mat4 transformation = glm::mat4(1.0f);

    CameraNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up_) 
                                : Node(n, pos), direction(glm::normalize(dir)), up(glm::normalize(up_)) {
        glm::vec3 xaxis = glm::normalize(glm::cross(up, direction));
        up = glm::normalize(glm::cross(direction, xaxis));
    }
    
    std::string getType() const { return type; } 
    virtual void update() {}
    virtual void render() {}
    virtual void setup() {}
};

class PersCameraNode : public CameraNode {
public:
    std::string type = "PersCameraNode";

    float fovy;
    float zNear;
    float zFar;
    float aspect;

    PersCameraNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float f, float zN, float zF, float as)
                                : CameraNode(n, pos, dir, up), fovy(f), zNear(zN), zFar(zF), aspect(as) {}

    std::string getType() const { return type; } 
    virtual void update();
    virtual void render();
    virtual void setup();
};

class OrthoCameraNode : public CameraNode {
public:
    std::string type = "OrthoCameraNode";

    float half_width;
    float half_height;
    float zFar;

    OrthoCameraNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float f, float distance, float zF)
                                : CameraNode(n, pos, dir, up), half_width(distance*tan(f*glm::pi<float>()/180.0f)* 500.0f/750.0f), half_height(distance*tan(f*glm::pi<float>()/180.0f)), zFar(zF) {}
                                

    std::string getType() const { return type; } 
    virtual void update();
    virtual void render();
    virtual void setup();
};
//__________ Scene __________

class SceneNode : public Node {
protected:
    std::vector<std::shared_ptr<CollisionNode>> collisions;
    void kill();
    void detach();
    void collide();
    void collectCollisions();

public:
    std::string type = "SceneNode";

    //ObjectManager objectManager;

    std::shared_ptr<CameraNode> main_camera;

    std::shared_ptr<ObjectNode> axes;

    SceneNode(const std::string& n);

    std::string getType() const { return type; } 
    virtual void update();
    virtual void render();
    void render(const glm::mat4& m);
    
    void push_collision(const std::shared_ptr<CollisionNode>& c) { collisions.push_back(c); }
};

// __________ Object __________

class ObjectNode : public Node {
public:
    bool independent = false;
    bool toDelete = false;

    std::string type = "ObjectNode";

    ObjectNode(const std::string& n, const glm::vec3& pos, const glm::quat& rot);
    ObjectNode(const std::string& n, const glm::vec3& pos, const glm::vec3 dir);

    void destroy() { invisible = true; inactive = true; toDelete = true; }

    std::string getType() const { return type; } 
    virtual void update();
    virtual void render();

    virtual void attacked(ObjectNode* obj) { return; }
};

// __________ Moodel __________

class ModelNode : public Node {
public:
    std::string type = "ModelNode";
    
    float scale;
    glm::vec4 color;
    
    ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec4& c, const std::string& t="", const std::string& nt="");
    ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec3& c, const std::string& t="", const std::string& nt="");
    ModelNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, float s, const glm::vec4& c, const std::string& t="", const std::string& nt="");
    ModelNode(const std::string& n, const glm::vec3& pos, const glm::vec3& dir, float s, const glm::vec3& c, const std::string& t="", const std::string& nt="");
    ModelNode(const std::string& n, const glm::vec3& pos, const glm::quat& q, float s, const glm::vec3& c, const std::string& t, GLuint vbo);

    std::string getType() const { return type; } 
    void update();
    void render();
};

// __________ Collision __________

class CollisionNode : public Node {
public:
    ObjectNode* parent;
    glm::vec3 real_position;    

    std::string type = "CollisionNode";

    float radius;

    CollisionNode(const std::string& n, const glm::vec3& pos, float r, ObjectNode* p);

    std::string getType() const { return type; } 
    void update();
    void render();
};