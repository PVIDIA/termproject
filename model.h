#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>


class Model {
public:
    std::string name;
    GLuint vao;
    GLuint vbo;
    GLuint count;
    std::string texture;
    std::string normal_texture;

    Model() : name(""), texture(""), normal_texture("") {}
    Model(const std::string&, const std::string&, const std::string&);

    void draw() const;
};

Model loadModelData(const std::string&, const std::string&);

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