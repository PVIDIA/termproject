#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>

// Material properties for a mesh
struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    std::string diffuseTexture;
    std::string normalTexture;
    
    Material() : ambient(0.2f), diffuse(0.8f), specular(0.5f), shininess(32.0f) {}
};

// Single mesh within a model
struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint count;
    Material material;
    
    Mesh() : vao(0), vbo(0), count(0) {}
};

class Model {
public:
    std::string name;
    std::vector<Mesh> meshes;
    
    // Legacy compatibility - references first mesh
    GLuint vao;
    GLuint vbo;
    GLuint count;
    std::string texture;
    std::string normal_texture;
    
    // For future animation support
    const aiScene* scene;  // Keep scene for bone/animation data later
    std::vector<glm::mat4> boneTransforms;  // Reserved for skeletal animation

    Model() : name(""), texture(""), normal_texture(""), vao(0), vbo(0), count(0), scene(nullptr) {}
    Model(const std::string&, const std::string&, const std::string&);

    void draw() const;
};

Model loadModelData(const std::string&, const std::string&);
Model loadModelAssimp(const std::string&, const std::string&, bool flipUVs = true);

class ModelManager {
public:
    std::unordered_map<std::string, Model> models;
    std::unordered_map<std::string, GLuint> textures;

    ModelManager() {}

    int count(const std::string& key) { return models.count(key); }
    Model& at(const std::string& key) { return models.at(key); }
    void draw_model(const std::string& key) { models.at(key).draw(); }
    GLuint get_texture(const std::string& key) { return textures[key]; }
    std::string loadModel(const std::string& n, const std::string& t="", const std::string& nt="");

};

GLuint loadTexture(const std::string& path);